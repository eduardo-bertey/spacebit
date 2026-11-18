// App Rust que embebe MatRiCT-Au (C) por FFI.
// Transacción con montos elegidos (billete de 1000, pago de 456) y un
// texto firmado dentro del hash del desafío.

use spacebit_au::demo_pago;

fn main() {
    let r = demo_pago(
        "certificado: el trabajo 42 lo hizo el peer dueno de este billete",
        "certificado: el trabajo 42 lo hizo otro peer",
        1000,
        456,
    )
    .expect("la ronda con montos validos debe completarse");

    println!("billete gastado: 1000 · pago: {} · vuelto: {}", r.amt0_real, r.amt1_real);
    println!(
        "verificación con el texto correcto: {}",
        if r.verify_ok == 1 { "OK" } else { "FALLA" }
    );
    println!(
        "cuenta real que gastó: {} · lo que abrió el observador: cuenta {}, montos {} y {}",
        r.l_real, r.l_audit, r.amt0_audit, r.amt1_audit
    );
    println!(
        "verificación con el texto alterado: {}",
        if r.verify_alterado == 1 { "OK (mal)" } else { "SE CAE (correcto)" }
    );
}
