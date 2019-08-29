printf "creating files\n"
rm -rf tmp/
mkdir tmp
for i in {1..10}; do
  mkdir "tmp/a$i"
  for j in {1..100}; do
    mkdir "tmp/a$i/b$j"
    for k in {1..50}; do
      touch "tmp/a$i/b$j/$k.tmp"
    done
  done
done

avg_time() {
    #
    # usage: avg_time n command ...
    #
    n=$1; shift
    (($# > 0)) || return                   # bail if no command given
    for ((i = 0; i < n; i++)); do
        { /usr/bin/time -p "$@"; } 2>&1
                                           # but collect time's output in stdout
    done | awk '
        /real/ { real = real + $2; nr++ }
        /user/ { user = user + $2; nu++ }
        /sys/  { sys  = sys  + $2; ns++}
        END    {
                 printf("%.3fs | %.3fs | %.3fs |\n", user/nu, sys/ns, sys/ns + user/nu);
               }'
}

printf "| Test                     | User   | System | Total  |\n"
printf "| ------------------------ | ------ | ------ | ------ |\n"

clang -O3 benchmark_recursive_cf.c -o benchmark_recursive_cf
printf "| Cute Files               | "
avg_time 100 ./benchmark_recursive_cf

clang -O3 benchmark_recursive_tinydir.c -o benchmark_recursive_tinydir
printf "| TinyDir                  | "
avg_time 100 ./benchmark_recursive_tinydir

printf "| Python (os.walk)         | "
avg_time 100 python3 ./benchmark_recursive.py ./tmp

clang -O3 benchmark_recursive.c -o benchmark_recursive
printf "| CPath (Recursion in C)   | "
avg_time 100 ./benchmark_recursive

clang -O3 benchmark_stack.c -o benchmark_stack
printf "| CPath (Emplace in C)     | "
avg_time 100 ./benchmark_stack

clang -O3 benchmark_recursive.cpp -o benchmark_recursive_cpp
printf "| CPath (Recursive in cpp) | "
avg_time 100 ./benchmark_recursive_cpp

clang -O3 benchmark_stack.cpp -o benchmark_stack_cpp
printf "| CPath (Emplace in cpp)   | "
avg_time 100 ./benchmark_stack_cpp

printf "| tree                     | "
avg_time 100 tree . -o /dev/null

printf "| find                     | "
avg_time 100 find .
