#/bin/sh

mkdir -p data/empty_dir
mkdir -p data/2files_dir
touch data/2files_dir/one
touch data/2files_dir/two

./test-io-dir-list || exit 1

rm data/2files_dir/one data/2files_dir/two
rmdir data/empty_dir data/2files_dir