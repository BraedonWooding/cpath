# printf "creating files\n"
# rm -rf tmp/
# mkdir tmp
# for i in {1..5}; do
#   mkdir "tmp/a$i"
#   for j in {1..100}; do
#     mkdir "tmp/a$i/b$j"
#     for k in {1..40}; do
#       touch "tmp/a$i/b$j/$k.tmp"
#     done
#   done
# done

printf "Testing python: "
time python3 ./benchmark_recursive.py ./tmp > /dev/null

clang -O0 benchmark_recursive.c -o benchmark_recursive
printf "Testing cpath_recursion: "
time ./benchmark_recursive > /dev/null

clang -O0 benchmark_stack.c -o benchmark_stack
printf "Testing cpath_stack: "
time ./benchmark_stack > /dev/null

printf "Testing tree: "
time tree . > /dev/null

printf "Testing find: "
time find . > /dev/null
