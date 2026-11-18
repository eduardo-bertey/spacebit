use spacebit_au::{
    demo_pago, prueba_firma_falsa, prueba_fuera_rango, prueba_serial_cero,
};
use std::sync::Mutex;

// El shim C usa buffers estaticos (g, h, g_hat, texto extra). Los tests de
// integration corren en paralelo, asi que los serializamos.
static LOCK: Mutex<()> = Mutex::new(());

const TEXTO: &str = "certificado: el trabajo 42 lo hizo el peer dueno de este billete";
const ALTERADO: &str = "certificado: el trabajo 42 lo hizo otro peer";

#[test]
fn pago_honesto_cuadra_con_la_auditoria() {
    let _g = LOCK.lock().unwrap();
    let r = demo_pago(TEXTO, ALTERADO, 1000, 456).unwrap();

    assert_eq!(r.verify_ok, 1);
    assert_eq!(r.verify_alterado, 0); // el texto alterado debe romper el hash
    assert_eq!(r.amt0_real, 456);
    assert_eq!(r.amt1_real, 544);
    assert!(r.audit_cuadra(), "observador: {:?}", r);
}

#[test]
fn montos_distintos() {
    let _g = LOCK.lock().unwrap();
    const CASOS: [(u64, u64); 4] = [
        (1000, 1),
        (1000, 999),
        (777_777_777, 123),
        (9_223_372_036_854_775_000, 9_223_372_036_854_774_999),
    ];

    for (billete, pago) in CASOS {
        let r = demo_pago(TEXTO, ALTERADO, billete, pago).unwrap();
        assert_eq!(r.verify_ok, 1, "billete={billete} pago={pago}");
        assert_eq!(r.verify_alterado, 0, "billete={billete} pago={pago}");
        assert_eq!(r.amt0_real, pago);
        assert_eq!(r.amt1_real, billete - pago);
        assert!(r.audit_cuadra(), "caso ({billete},{pago}): {:?}", r);
    }
}

#[test]
fn pago_mayor_que_el_billete_se_rechaza() {
    let _g = LOCK.lock().unwrap();
    assert_eq!(demo_pago(TEXTO, ALTERADO, 100, 101), Err(-1));
}

#[test]
fn parche_imagen_cero_rechaza_el_serial_cero() {
    let _g = LOCK.lock().unwrap();
    let (normal, cero) = prueba_serial_cero();
    assert_eq!(normal, 1, "la prueba honesta debe pasar");
    assert_eq!(cero, 0, "el parche imagen-cero debe rechazar");
}

#[test]
fn prueba_firmada_alterada_se_cae() {
    let _g = LOCK.lock().unwrap();
    let (normal, falso) = prueba_firma_falsa();
    assert_eq!(normal, 1);
    assert_eq!(falso, 0, "alterar zc despues de firmar debe fallar");
}

#[test]
fn firmante_fuera_del_anillo_se_cae() {
    let _g = LOCK.lock().unwrap();
    let (normal, falso) = prueba_fuera_rango();
    assert_eq!(normal, 1);
    assert_eq!(falso, 0, "el verificador no debe aceptar otro anillo");
}
