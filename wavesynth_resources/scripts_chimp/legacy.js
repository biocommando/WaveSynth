// Legacy question mark assign notation.
// Use example:
// lgc Bank_? = 3
// The question mark will be substituted with the value of variable 'index'.
// This works only with simple assignments.
'macro lgc'
str => {
    const m = str.match(/[^\s]*\?/g)
    if (!m) return str

    const a = [], b = []
    m.forEach(x => {
        a.push(
            `hint::PREPROC_START set parameter to ${x}`,
            `if_then_ignore_warning index == 1`,
            `${x} = ${x.replace('?', '1')}`,
            `if_then_ignore_warning index == 2`,
            `${x} = ${x.replace('?', '2')}`,
            `if_then_ignore_warning index == 3`,
            `${x} = ${x.replace('?', '3')}`,
            `hint::PREPROC_END set parameter to ${x}`,
        )
        b.push(
            `hint::PREPROC_START set result to ${x}`,
            `if_then_ignore_warning index == 1`,
            `${x.replace('?', '1')} = ${x}`,
            `if_then_ignore_warning index == 2`,
            `${x.replace('?', '2')} = ${x}`,
            `if_then_ignore_warning index == 3`,
            `${x.replace('?', '3')} = ${x}`,
            `hint::PREPROC_END set result to ${x}`,
        )
    })
    return [
        ...a, str, ...b
    ]
}
'macro end'

// Shorthand for lgc
'macro %'
macros.lgc
'macro end'

// Example: resolve_question_mark Bank_?
// This command will get the value of index (e.g. 2)
// And replace the line of code with the following assignation:
// Bank_? = Bank_2
'macro resolve_question_mark'
x => [
    `if_then_ignore_warning index == 1`,
    `${x} = ${x.replace('?', '1')}`,
    `if_then_ignore_warning index == 2`,
    `${x} = ${x.replace('?', '2')}`,
    `if_then_ignore_warning index == 3`,
    `${x} = ${x.replace('?', '3')}`
]
'macro end'

// Notify the script engine that all three parameters that can be
// associated with the question mark notation are handled as integers
'macro params_as_integer'
str => {
    let params = []
    str.split(',').forEach(x => {
        x = x.trim()
        if (x.includes('?')) {
            params.push(x.replace('?', 1))
            params.push(x.replace('?', 2))
            params.push(x.replace('?', 3))
        } else {
            params.push(x)
        }
    })
    
    return [
        `notify "params_are_integers", ${params.map(x => '"' + x + '"').join(', ')}`
    ]
}
'macro end'


