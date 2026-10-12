// CLAP entry point & plugin implementation for WaveSynth

#include <clap/clap.h>
#include <cstring>
#include <cstdio>
#include <atomic>
#include <map>
#include <thread>
#include <vector>
#include <windows.h>

#include "WaveSynth.h"
#include "EditorGui.h"

static const char *const PLUGIN_ID = "com.joonassalonpaa.wavesynth";

// Debug logging into %TEMP%\wavesynth-clap.log, enabled with -DCLAP_DEBUG_LOG
#ifdef CLAP_DEBUG_LOG
#include <cstdarg>
// Always appends (a reload must not wipe the previous instance's teardown). Delete the file to start over.
static void dlog(const char *fmt, ...)
{
	char path[MAX_PATH];
	DWORD n = GetTempPathA(sizeof(path), path);
	if (n == 0 || n + 20 > sizeof(path))
		return;
	strcat(path, "wavesynth-clap.log");
	FILE *f = fopen(path, "a");
	if (!f)
		return;
	SYSTEMTIME t;
	GetLocalTime(&t);
	fprintf(f, "%02d:%02d:%02d.%03d [%lu] ", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds, GetCurrentThreadId());
	va_list args;
	va_start(args, fmt);
	vfprintf(f, fmt, args);
	va_end(args);
	fputc('\n', f);
	fclose(f);
}

// VSTGui's window class name ("Plugin%p" with hInstance) and whether it still points into this DLL.
// A VSTGui window that outlives the DLL crashes the host in VSTGUI::WindowProc after unload.
static void dlog_vstgui_windows(const char *when)
{
	extern void *hInstance;
	if (!hInstance)
	{
		dlog("%s: hInstance not set (no GUI opened)", when);
		return;
	}
	wchar_t cls[100];
	wsprintfW(cls, L"Plugin%p", hInstance);
	WNDCLASSEXW wc = {sizeof(wc)};
	bool registered = GetClassInfoExW((HINSTANCE)hInstance, cls, &wc) != 0;
	HMODULE owner = nullptr;
	if (registered)
		GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
						   (LPCWSTR)wc.lpfnWndProc, &owner);
	HMODULE self = nullptr;
	GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
					   (LPCWSTR)&dlog_vstgui_windows, &self);
	struct Ctx
	{
		const wchar_t *cls;
		int count;
	} ctx = {cls, 0};
	// Child windows of all top-level windows of this process
	EnumWindows([](HWND top, LPARAM lp) -> BOOL {
		DWORD pid = 0;
		GetWindowThreadProcessId(top, &pid);
		if (pid != GetCurrentProcessId())
			return TRUE;
		EnumChildWindows(top, [](HWND w, LPARAM lp) -> BOOL {
			auto c = (Ctx *)lp;
			wchar_t name[100];
			if (GetClassNameW(w, name, 100) && !wcscmp(name, c->cls))
			{
				c->count++;
				dlog("    VSTGui window %p alive, thread %lu, visible %d", (void *)w,
					 GetWindowThreadProcessId(w, nullptr), (int)IsWindowVisible(w));
			}
			return TRUE;
		}, lp);
		return TRUE;
	}, (LPARAM)&ctx);
	dlog("%s: VSTGui class %s%s, %d VSTGui window(s) alive", when,
		 registered ? "registered" : "not registered",
		 registered ? (owner == self ? " (window proc in THIS dll)" : " (window proc in another module)") : "",
		 ctx.count);
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
		dlog("==== DLL loaded");
	else if (reason == DLL_PROCESS_DETACH)
	{
		dlog("==== DLL unloading (%s)", reserved ? "process exit" : "FreeLibrary");
		if (!reserved)
			dlog_vstgui_windows("at unload");
	}
	return TRUE;
}
#else
#define dlog(...)
#define dlog_vstgui_windows(...)
#endif

void *hInstance; // Required by VSTGui. Set like in clapwrap: the host exe's module handle, set in gui create.

// The plugin file path given by the host in clap_entry.init (same as in clapwrap)
static char _plugin_path[1024] = "";
static void set_plugin_path(const char *path)
{
	if (strlen(path) >= sizeof(_plugin_path))
		return;
	strcpy(_plugin_path, path);
}

const char *get_plugin_path()
{
	return _plugin_path;
}

static const char *plugin_features[] = {
	CLAP_PLUGIN_FEATURE_INSTRUMENT,
	CLAP_PLUGIN_FEATURE_SYNTHESIZER,
	CLAP_PLUGIN_FEATURE_STEREO,
	nullptr};

static const clap_plugin_descriptor_t plugin_descriptor = {
	CLAP_VERSION_INIT,
	PLUGIN_ID,
	"Little, ugly gnome",
	"Joonas Salonpaa",
	"https://github.com/biocommando",
	"",
	"",
	"2.0.0",
	"Wavetable synth",
	plugin_features,
};

struct Plugin
{
	clap_plugin_t clap;
	const clap_host_t *host;
	const clap_host_params_t *host_params;
	WaveSynth *synth;
	EditorGui *editor;
	UINT_PTR gui_timer;
	// FL Studio calls start_processing/process on another thread before activate has returned.
	// activate rebuilds the DSP and loads waves, so process must not touch the synth meanwhile:
	// process runs the synth only when active, and activate/deactivate/destroy wait for an
	// in-flight process call to finish. (seq_cst atomics: each side stores its flag, then reads the other's.)
	std::atomic<bool> active{false};
	std::atomic<bool> in_process{false};
#ifdef CLAP_DEBUG_LOG
	bool logged_first_process = false;
#endif
};

// Stops processing from touching the synth and waits until a process call in progress has returned
static void stop_audio_thread_access(Plugin *p)
{
	p->active = false;
	while (p->in_process)
		std::this_thread::yield();
}

static Plugin *get(const clap_plugin_t *plugin)
{
	return (Plugin *)plugin->plugin_data;
}

/// Events

static void handle_event(Plugin *p, const clap_event_header_t *ev)
{
	if (ev->space_id != CLAP_CORE_EVENT_SPACE_ID)
		return;

	switch (ev->type)
	{
	case CLAP_EVENT_PARAM_VALUE:
	{
		auto pe = (const clap_event_param_value_t *)ev;
		if (pe->param_id < NUM_PARAMS)
			p->synth->setParameter(pe->param_id, (float)pe->value);
		break;
	}
	case CLAP_EVENT_NOTE_ON:
	{
		auto ne = (const clap_event_note_t *)ev;
		if (ne->key < 0)
			break;
		int velocity = (int)(ne->velocity * 127 + 0.5);
		if (velocity < 1)
			velocity = 1;
		const uint8_t midi[3] = {0x90, (uint8_t)ne->key, (uint8_t)velocity};
		p->synth->processMidi(midi);
		break;
	}
	case CLAP_EVENT_NOTE_OFF:
	case CLAP_EVENT_NOTE_CHOKE:
	{
		auto ne = (const clap_event_note_t *)ev;
		if (ne->key < 0)
			break;
		const uint8_t midi[3] = {0x80, (uint8_t)ne->key, 0};
		p->synth->processMidi(midi);
		break;
	}
	case CLAP_EVENT_MIDI:
	{
		auto me = (const clap_event_midi_t *)ev;
		p->synth->processMidi(me->data);
		break;
	}
	}
}

// Report parameter changes made from GUI / MIDI CC mappings to the host
static void send_pending_param_changes(Plugin *p, const clap_output_events_t *out)
{
	if (!out)
		return;
	for (int index : p->synth->takePendingHostNotifications())
	{
		clap_event_param_value_t ev = {};
		ev.header.size = sizeof(ev);
		ev.header.time = 0;
		ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
		ev.header.type = CLAP_EVENT_PARAM_VALUE;
		ev.header.flags = 0;
		ev.param_id = index;
		ev.cookie = nullptr;
		ev.note_id = -1;
		ev.port_index = -1;
		ev.channel = -1;
		ev.key = -1;
		ev.value = p->synth->getParameter(index);
		out->try_push(out, &ev.header);
	}
}

/// Plugin

static bool plugin_init(const clap_plugin_t *plugin)
{
	auto p = get(plugin);
	dlog("init: host '%s' version '%s'", p->host->name ? p->host->name : "", p->host->version ? p->host->version : "");
	p->host_params = (const clap_host_params_t *)p->host->get_extension(p->host, CLAP_EXT_PARAMS);
	p->synth->init();
	dlog("init done, use gui %d", (int)p->synth->getUseGUI());
	return true;
}

static void plugin_destroy(const clap_plugin_t *plugin)
{
	auto p = get(plugin);
	dlog("plugin destroy, editor %p (gui destroy not called if not null)", (void *)p->editor);
	stop_audio_thread_access(p);
	delete p->synth;
	delete p;
}

static bool plugin_activate(const clap_plugin_t *plugin, double sample_rate, uint32_t, uint32_t)
{
	auto p = get(plugin);
	dlog("activate %f", sample_rate);
	stop_audio_thread_access(p);
	p->synth->activate((int)sample_rate);
#ifdef CLAP_DEBUG_LOG
	p->logged_first_process = false;
#endif
	p->active = true;
	dlog("activate done");
	return true;
}

static void plugin_deactivate(const clap_plugin_t *plugin)
{
	dlog("deactivate");
	stop_audio_thread_access(get(plugin));
}

static bool plugin_start_processing(const clap_plugin_t *)
{
	dlog("start_processing");
	return true;
}

static void plugin_stop_processing(const clap_plugin_t *)
{
	dlog("stop_processing");
}

static void plugin_reset(const clap_plugin_t *) {}

static clap_process_status process_active(Plugin *p, const clap_process_t *process);

static clap_process_status plugin_process(const clap_plugin_t *plugin, const clap_process_t *process)
{
	auto p = get(plugin);
	p->in_process = true;
	if (!p->active)
	{
		// Called before activate has finished (FL does this): output silence, don't touch the synth
		for (uint32_t i = 0; i < process->audio_outputs_count; i++)
			for (uint32_t ch = 0; ch < process->audio_outputs[i].channel_count; ch++)
				memset(process->audio_outputs[i].data32[ch], 0, process->frames_count * sizeof(float));
		p->in_process = false;
		return CLAP_PROCESS_CONTINUE;
	}
	auto status = process_active(p, process);
	p->in_process = false;
	return status;
}

static clap_process_status process_active(Plugin *p, const clap_process_t *process)
{
	auto synth = p->synth;
#ifdef CLAP_DEBUG_LOG
	// Only the first block per activation: file I/O doesn't belong on the audio thread
	if (!p->logged_first_process)
	{
		p->logged_first_process = true;
		dlog("first process: %u frames, %u events", process->frames_count,
			 process->in_events->size(process->in_events));
	}
#endif

	if (process->transport)
	{
		const auto flags = process->transport->flags;
		synth->setTransportState((flags & CLAP_TRANSPORT_IS_PLAYING) != 0,
								 (flags & CLAP_TRANSPORT_HAS_TEMPO) != 0, process->transport->tempo);
	}

	if (process->audio_outputs_count < 1 || process->audio_outputs[0].channel_count < 2)
		return CLAP_PROCESS_ERROR;

	float *outputs[2] = {process->audio_outputs[0].data32[0], process->audio_outputs[0].data32[1]};
	const uint32_t frames = process->frames_count;
	const uint32_t num_events = process->in_events->size(process->in_events);

	// Render audio in slices between events so that the events are sample accurate
	uint32_t event_idx = 0, pos = 0;
	while (pos < frames)
	{
		uint32_t next_pos = frames;
		while (event_idx < num_events)
		{
			auto ev = process->in_events->get(process->in_events, event_idx);
			if (ev->time > pos)
			{
				next_pos = ev->time < frames ? ev->time : frames;
				break;
			}
			handle_event(p, ev);
			event_idx++;
		}

		float *slice[2] = {outputs[0] + pos, outputs[1] + pos};
		synth->processReplacing(slice, next_pos - pos);
		pos = next_pos;
	}
	// Events that have a timestamp past the buffer end
	for (; event_idx < num_events; event_idx++)
		handle_event(p, process->in_events->get(process->in_events, event_idx));

	send_pending_param_changes(p, process->out_events);

	return CLAP_PROCESS_CONTINUE;
}

/// Params

static uint32_t params_count(const clap_plugin_t *)
{
	return NUM_PARAMS;
}

static bool params_get_info(const clap_plugin_t *plugin, uint32_t index, clap_param_info_t *info)
{
	if (index >= NUM_PARAMS)
		return false;
	auto synth = get(plugin)->synth;
	memset(info, 0, sizeof(*info));
	info->id = index;
	info->flags = CLAP_PARAM_IS_AUTOMATABLE;
	info->cookie = nullptr;
	synth->getParameterName(index, info->name);
	if (index < TOTAL_SET_PARAMS_COUNT)
		snprintf(info->module, sizeof(info->module), "Osc %d", index / SET_PARAMS_COUNT + 1);
	else
		snprintf(info->module, sizeof(info->module), "Global");
	info->min_value = 0;
	info->max_value = 1;
	info->default_value = synth->getParameterDefault(index);
	return true;
}

static bool params_get_value(const clap_plugin_t *plugin, clap_id id, double *value)
{
	if (id >= NUM_PARAMS)
		return false;
	*value = get(plugin)->synth->getParameter(id);
	return true;
}

static bool params_value_to_text(const clap_plugin_t *plugin, clap_id id, double value, char *display, uint32_t size)
{
	if (id >= NUM_PARAMS)
		return false;
	get(plugin)->synth->getParameterDisplay(id, value, display, size);
	return true;
}

static bool params_text_to_value(const clap_plugin_t *, clap_id, const char *, double *)
{
	return false;
}

static void params_flush(const clap_plugin_t *plugin, const clap_input_events_t *in, const clap_output_events_t *out)
{
	auto p = get(plugin);
	const uint32_t n = in->size(in);
	for (uint32_t i = 0; i < n; i++)
	{
		auto ev = in->get(in, i);
		// Notes are only processed in process()
		if (ev->space_id == CLAP_CORE_EVENT_SPACE_ID && ev->type == CLAP_EVENT_PARAM_VALUE)
			handle_event(p, ev);
	}
	send_pending_param_changes(p, out);
}

static const clap_plugin_params_t params_ext = {
	params_count,
	params_get_info,
	params_get_value,
	params_value_to_text,
	params_text_to_value,
	params_flush,
};

/// State

static bool state_save(const clap_plugin_t *plugin, const clap_ostream_t *stream)
{
	const auto chunk = get(plugin)->synth->getChunk();
	size_t written = 0;
	while (written < chunk.size())
	{
		int64_t r = stream->write(stream, chunk.data() + written, chunk.size() - written);
		if (r <= 0)
			return false;
		written += r;
	}
	return true;
}

static bool state_load(const clap_plugin_t *plugin, const clap_istream_t *stream)
{
	auto p = get(plugin);
	std::vector<char> buffer;
	char temp[4096];
	for (;;)
	{
		int64_t r = stream->read(stream, temp, sizeof(temp));
		if (r < 0)
			return false;
		if (r == 0)
			break;
		buffer.insert(buffer.end(), temp, temp + r);
	}
	if (buffer.empty())
		return false;

	if (!p->synth->setChunk(buffer.data(), (int)buffer.size()))
		return false;

	if (p->editor)
	{
		for (int i = 0; i < NUM_PARAMS; i++)
			p->editor->setParameter(i, p->synth->getParameter(i));
	}
	if (p->host_params)
		p->host_params->rescan(p->host, CLAP_PARAM_RESCAN_VALUES | CLAP_PARAM_RESCAN_TEXT);
	return true;
}

static const clap_plugin_state_t state_ext = {
	state_save,
	state_load,
};

/// Audio & note ports

static uint32_t audio_ports_count(const clap_plugin_t *, bool is_input)
{
	return is_input ? 0 : 1;
}

static bool audio_ports_get(const clap_plugin_t *, uint32_t index, bool is_input, clap_audio_port_info_t *info)
{
	if (is_input || index > 0)
		return false;
	info->id = 0;
	snprintf(info->name, sizeof(info->name), "Output");
	info->channel_count = 2;
	info->flags = CLAP_AUDIO_PORT_IS_MAIN;
	info->port_type = CLAP_PORT_STEREO;
	info->in_place_pair = CLAP_INVALID_ID;
	return true;
}

static const clap_plugin_audio_ports_t audio_ports_ext = {
	audio_ports_count,
	audio_ports_get,
};

static uint32_t note_ports_count(const clap_plugin_t *, bool is_input)
{
	return is_input ? 1 : 0;
}

static bool note_ports_get(const clap_plugin_t *, uint32_t index, bool is_input, clap_note_port_info_t *info)
{
	if (!is_input || index > 0)
		return false;
	info->id = 0;
	snprintf(info->name, sizeof(info->name), "Note Input");
	info->supported_dialects = CLAP_NOTE_DIALECT_CLAP | CLAP_NOTE_DIALECT_MIDI;
	// MIDI dialect gives also pitch bend, CC & aftertouch
	info->preferred_dialect = CLAP_NOTE_DIALECT_MIDI;
	return true;
}

static const clap_plugin_note_ports_t note_ports_ext = {
	note_ports_count,
	note_ports_get,
};

/// GUI

static std::map<UINT_PTR, Plugin *> gui_timers;

static void CALLBACK gui_timer_callback(HWND, UINT, UINT_PTR id, DWORD)
{
	auto it = gui_timers.find(id);
	if (it == gui_timers.end())
		return;
	auto p = it->second;
	if (p->editor)
		p->editor->idle();
	// Ask the host to collect GUI parameter changes even if the audio processing is not running
	if (p->host_params && p->synth->hasPendingHostNotifications())
		p->host_params->request_flush(p->host);
}

static bool gui_is_api_supported(const clap_plugin_t *, const char *api, bool is_floating)
{
	dlog("gui is_api_supported %s floating %d", api ? api : "(null)", (int)is_floating);
	return api && !strcmp(api, CLAP_WINDOW_API_WIN32) && !is_floating;
}

static bool gui_get_preferred_api(const clap_plugin_t *, const char **api, bool *is_floating)
{
	*api = CLAP_WINDOW_API_WIN32;
	*is_floating = false;
	return true;
}

static bool gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating)
{
	dlog("gui create %s floating %d", api ? api : "(null)", (int)is_floating);
	if (!gui_is_api_supported(plugin, api, is_floating))
		return false;
	auto p = get(plugin);
	if (p->editor)
		return true;
	p->editor = new EditorGui(p->synth);
	p->synth->editor = p->editor;
	hInstance = GetModuleHandle(nullptr);
	p->gui_timer = SetTimer(nullptr, 0, 33, gui_timer_callback);
	gui_timers[p->gui_timer] = p;
	return true;
}

static void gui_destroy(const clap_plugin_t *plugin)
{
	dlog("gui destroy");
	auto p = get(plugin);
	if (p->gui_timer)
	{
		KillTimer(nullptr, p->gui_timer);
		gui_timers.erase(p->gui_timer);
		p->gui_timer = 0;
	}
	if (p->editor)
	{
		auto editor = p->editor;
		p->synth->editor = nullptr;
		p->editor = nullptr;
		editor->close();
		delete editor;
	}
	dlog_vstgui_windows("after gui destroy");
}

static bool gui_set_scale(const clap_plugin_t *, double scale)
{
	dlog("gui set_scale %f", scale);
	return false;
}

static bool gui_get_size(const clap_plugin_t *, uint32_t *width, uint32_t *height)
{
	int w, h;
	EditorGui::getEditorSize(&w, &h);
	*width = w;
	*height = h;
	dlog("gui get_size -> %dx%d", w, h);
	return true;
}

static bool gui_can_resize(const clap_plugin_t *)
{
	dlog("gui can_resize");
	return false;
}

static bool gui_get_resize_hints(const clap_plugin_t *, clap_gui_resize_hints_t *)
{
	return false;
}

static bool gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height)
{
	dlog("gui adjust_size %ux%u", *width, *height);
	return gui_get_size(plugin, width, height);
}

static bool gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height)
{
	uint32_t w, h;
	gui_get_size(plugin, &w, &h);
	dlog("gui set_size %ux%u", width, height);
	return w == width && h == height;
}

static bool gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window)
{
	auto p = get(plugin);
	dlog("gui set_parent %p, editor %p", window->win32, (void *)p->editor);
	if (!p->editor)
		return false;
	if (!p->editor->open(window->win32))
		return false;
	dlog("gui set_parent: opened, frame %p", (void *)p->editor->getFrame());
	return true;
}

static bool gui_set_transient(const clap_plugin_t *, const clap_window_t *)
{
	dlog("gui set_transient");
	return false;
}

static void gui_suggest_title(const clap_plugin_t *, const char *) {}

static bool gui_show(const clap_plugin_t *plugin)
{
	auto p = get(plugin);
	dlog("gui show, editor %p frame %p", (void *)p->editor, p->editor ? (void *)p->editor->getFrame() : nullptr);
	if (!p->editor || !p->editor->getFrame())
		return false;
	p->editor->getFrame()->setVisible(true);
	return true;
}

static bool gui_hide(const clap_plugin_t *plugin)
{
	auto p = get(plugin);
	dlog("gui hide");
	if (!p->editor || !p->editor->getFrame())
		return false;
	p->editor->getFrame()->setVisible(false);
	return true;
}

static const clap_plugin_gui_t gui_ext = {
	gui_is_api_supported,
	gui_get_preferred_api,
	gui_create,
	gui_destroy,
	gui_set_scale,
	gui_get_size,
	gui_can_resize,
	gui_get_resize_hints,
	gui_adjust_size,
	gui_set_size,
	gui_set_parent,
	gui_set_transient,
	gui_suggest_title,
	gui_show,
	gui_hide,
};

static const void *plugin_get_extension(const clap_plugin_t *plugin, const char *id)
{
	dlog("get_extension %s", id);
	if (!strcmp(id, CLAP_EXT_PARAMS))
		return &params_ext;
	if (!strcmp(id, CLAP_EXT_STATE))
		return &state_ext;
	if (!strcmp(id, CLAP_EXT_AUDIO_PORTS))
		return &audio_ports_ext;
	if (!strcmp(id, CLAP_EXT_NOTE_PORTS))
		return &note_ports_ext;
	if (!strcmp(id, CLAP_EXT_GUI) && get(plugin)->synth->getUseGUI())
		return &gui_ext;
	return nullptr;
}

static void plugin_on_main_thread(const clap_plugin_t *) {}

/// Factory & entry

static uint32_t factory_get_plugin_count(const clap_plugin_factory_t *)
{
	return 1;
}

static const clap_plugin_descriptor_t *factory_get_plugin_descriptor(const clap_plugin_factory_t *, uint32_t index)
{
	return index == 0 ? &plugin_descriptor : nullptr;
}

static const clap_plugin_t *factory_create_plugin(const clap_plugin_factory_t *, const clap_host_t *host, const char *plugin_id)
{
	if (!clap_version_is_compatible(host->clap_version) || strcmp(plugin_id, PLUGIN_ID))
		return nullptr;

	auto p = new Plugin();
	p->host = host;
	p->host_params = nullptr;
	p->synth = new WaveSynth();
	p->editor = nullptr;
	p->gui_timer = 0;

	p->clap.desc = &plugin_descriptor;
	p->clap.plugin_data = p;
	p->clap.init = plugin_init;
	p->clap.destroy = plugin_destroy;
	p->clap.activate = plugin_activate;
	p->clap.deactivate = plugin_deactivate;
	p->clap.start_processing = plugin_start_processing;
	p->clap.stop_processing = plugin_stop_processing;
	p->clap.reset = plugin_reset;
	p->clap.process = plugin_process;
	p->clap.get_extension = plugin_get_extension;
	p->clap.on_main_thread = plugin_on_main_thread;
	return &p->clap;
}

static const clap_plugin_factory_t plugin_factory = {
	factory_get_plugin_count,
	factory_get_plugin_descriptor,
	factory_create_plugin,
};

static bool entry_init(const char *path)
{
	dlog("entry_init path '%s'", path ? path : "(null)");
	set_plugin_path(path);
	return true;
}

static void entry_deinit()
{
	dlog("entry_deinit");
	dlog_vstgui_windows("at entry_deinit");
}

static const void *entry_get_factory(const char *factory_id)
{
	if (!strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID))
		return &plugin_factory;
	return nullptr;
}

extern "C" CLAP_EXPORT const clap_plugin_entry_t clap_entry = {
	CLAP_VERSION_INIT,
	entry_init,
	entry_deinit,
	entry_get_factory,
};
