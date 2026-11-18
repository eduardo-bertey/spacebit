use std::{env, fs, path::PathBuf};

fn main() {
    let manifest = PathBuf::from(env::var("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR"));
    let native = manifest.join("native");
    let au = native.join("au_src");
    let include = native.join("include");
    let portable_rng = manifest
        .parent()
        .expect("create/ debe estar dentro del repo")
        .join("test/randombytes_portable.c");

    assert!(au.exists(), "falta create/native/au_src/");
    assert!(native.join("shim.c").exists(), "falta create/native/shim.c");
    assert!(include.join("libXKCP.a.headers/SimpleFIPS202.h").exists(), "falta el header de SHAKE256");
    assert!(portable_rng.exists(), "falta test/randombytes_portable.c");

    println!("cargo:rerun-if-changed={}", native.display());
    println!("cargo:rerun-if-changed={}", portable_rng.display());

    if env::var("CARGO_CFG_TARGET_ENV").as_deref() == Ok("msvc") {
        println!(
            "cargo:warning=MSVC no compila el asm AT&T de au_src; usa --target x86_64-pc-windows-gnu"
        );
    }

    let mut build = cc::Build::new();
    build
        .include(&au)
        .include(&include)
        .include(&native)
        .flag_if_supported("-O2")
        .flag_if_supported("-maes");

    // Fuentes Au menos test.c (tiene main) y randombytes.c (usa /dev/urandom;
    // se sustituye por la version portable de test/).
    for entry in fs::read_dir(&au).expect("no se puede leer au_src/") {
        let path = entry.expect("entrada invalida").path();
        if path.extension().and_then(|s| s.to_str()) != Some("c") {
            continue;
        }
        let nombre = path.file_name().unwrap().to_string_lossy().to_string();
        if nombre == "test.c" || nombre == "randombytes.c" {
            continue;
        }
        build.file(path);
    }

    build.file(native.join("shake256.c")); // SHAKE-256 local, sin -lXKCP
    build.file(native.join("shim.c"));
    build.file(native.join("test_hooks.c")); // serial cero + falsificaciones
    build.file(portable_rng);

    build.compile("matrict_au");
}
