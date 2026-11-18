use std::{env, fs, path::PathBuf};

fn main() {
    let manifest = PathBuf::from(env::var("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR"));
    let root = manifest
        .parent()
        .expect("create_plus/ debe estar dentro del repo")
        .to_path_buf();
    let plus = root.join("plus");
    let include = root.join("create/native/include");
    let shake = root.join("create/native/shake256.c");
    let rng = root.join("test/randombytes_portable.c");
    let shim = manifest.join("native/shim.c");

    assert!(plus.exists(), "falta plus/");
    assert!(include.join("libXKCP.a.headers/SimpleFIPS202.h").exists(), "falta el header de SHAKE256");
    assert!(shake.exists(), "falta create/native/shake256.c");
    assert!(rng.exists(), "falta test/randombytes_portable.c");

    println!("cargo:rerun-if-changed={}", plus.display());
    println!("cargo:rerun-if-changed={}", shim.display());
    println!("cargo:rerun-if-changed={}", rng.display());

    if env::var("CARGO_CFG_TARGET_ENV").as_deref() != Ok("msvc") {
        println!("cargo:rustc-link-arg=-s");
    } else {
        println!("cargo:warning=MSVC no compila el asm/x86intrin del Plus; usa --target x86_64-pc-windows-gnu");
    }

    let mut build = cc::Build::new();
    build
        .include(&plus)
        .include(&include)
        .opt_level(3)
        .debug(false)
        .define("NDEBUG", None)
        .flag_if_supported("-maes")
        .flag_if_supported("-mavx2")
        .flag_if_supported("-mfma");

    // Fuentes Plus menos los main() de prueba y randombytes.c (se usa el RNG portable).
    const SKIP: &[&str] = &["test.c", "tp1.c", "keygen_zero.c", "randombytes.c"];
    for entry in fs::read_dir(&plus).expect("no se puede leer plus/") {
        let path = entry.expect("entrada invalida").path();
        if path.extension().and_then(|s| s.to_str()) != Some("c") {
            continue;
        }
        let nombre = path.file_name().unwrap().to_string_lossy().to_string();
        if SKIP.contains(&nombre.as_str()) {
            continue;
        }
        build.file(path);
    }

    build.file(shake);
    build.file(rng);
    build.file(manifest.join("native/shim.c"));
    build.compile("matrict_plus");
}
