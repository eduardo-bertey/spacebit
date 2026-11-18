// App Rust que embebe MatRiCT-Plus (C) por FFI.
use spacebit_plus::demo;

fn main() {
    let r = demo(1000, 500, 456).expect("montos validos");

    println!("=== PLUS (Rust) ===");
    println!("billetes: 1000 + 500 · pago: {} · vuelto: {}", r.pago, r.vuelto);
    println!("firmante: {} (anillo de 50)", r.l);
    println!("verificacion honesta: {}", if r.verify_ok == 1 { "OK" } else { "FALLA" });
    println!(
        "serial revelado a cero: {}",
        if r.verify_sn_cero == 0 { "RECHAZADO (correcto)" } else { "ACEPTADO (mal)" }
    );
    println!(
        "prueba alterada: {}",
        if r.verify_falso == 0 { "RECHAZADA (correcto)" } else { "ACEPTADA (mal)" }
    );
    println!(
        "ciclos CPU: setup={} firmar={} verificar={}",
        r.ciclos[0], r.ciclos[1], r.ciclos[2]
    );

    println!("\nFin. Presione Enter para salir...");
    use std::io::Write;
    let _ = std::io::stdout().flush();
    let mut s = String::new();
    let _ = std::io::stdin().read_line(&mut s);
}
