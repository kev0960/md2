fn main() {
    let md2_path = env!("CARGO_MANIFEST_DIR");
    println!("cargo:rustc-flags=-l dylib=stdc++ -l dylib=zmq -l dylib=fmt -l dylib=ssl -l dylib=crypto -l dylib=pqxx -l dylib=pq");

    // Sometimes md2_path points to host's cargo manifest dir, rather then container's dir.
    // TODO Figure out why this is happening.
    println!("{}", format!("cargo:rustc-link-search=/MathTeacher/md2/build/lib"));
    println!("{}", format!("cargo:rustc-link-search={}/build/lib", md2_path));
    println!("{}", format!("cargo:rerun-if-changed={}/build/lib", md2_path));
}
