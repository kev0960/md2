fn main() {
    println!("cargo:rustc-flags=-l dylib=stdc++ -l dylib=zmq -l dylib=fmt -l dylib=ssl -l dylib=crypto -l dylib=pqxx -l dylib=pq");
    println!("cargo:rustc-link-search=/home/jaebum/md2/build/lib");
}
