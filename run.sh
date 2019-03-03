#make
#./raytrace 1920 1080 24 3
#./raytrace 1024 768 24 3
time ./raytrace 3840 2160 24 3 10
cd ./render_output
./generate_movie.sh
