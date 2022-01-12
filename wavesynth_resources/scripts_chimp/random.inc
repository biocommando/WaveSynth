
inline sub random
    random = _random_
    random = (random % 10000) / 10000
return

sub generate_random_array
    i = isdef generate_random_array.max
    if_then_ignore_warning i == 0
        generate_random_array.max = 10
    for i in 0 to generate_random_array.max
        call random
        array_set random at i = random
    loop
return