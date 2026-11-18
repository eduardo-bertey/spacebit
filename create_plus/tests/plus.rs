use spacebit_plus::demo;
use std::sync::Mutex;

// El shim C usa buffers estaticos; serializamos los tests.
static LOCK: Mutex<()> = Mutex::new(());

#[test]
fn ronda_honestay_ataques_rechazados() {
    let _g = LOCK.lock().unwrap();
    let r = demo(1000, 500, 456).unwrap();

    assert_eq!(r.verify_ok, 1);
    assert_eq!(r.verify_sn_cero, 0, "el serial a cero debe rechazarse");
    assert_eq!(r.verify_falso, 0, "la prueba alterada debe rechazarse");
    assert_eq!(r.pago, 456);
    assert_eq!(r.vuelto, 1044);
}

#[test]
fn montos_distintos() {
    let _g = LOCK.lock().unwrap();
    const CASOS: [(u64, u64, u64); 4] = [
        (1000, 0, 1),
        (1000, 500, 999),
        (777_777_777, 123_456, 777_900_000),
        (9_223_372_036_854_775_000, 10, 9_223_372_036_854_774_999),
    ];

    for (a0, a1, pago) in CASOS {
        let r = demo(a0, a1, pago).unwrap();
        assert_eq!(r.verify_ok, 1, "caso ({a0},{a1},{pago})");
        assert_eq!(r.verify_sn_cero, 0, "caso ({a0},{a1},{pago})");
        assert_eq!(r.verify_falso, 0, "caso ({a0},{a1},{pago})");
        assert_eq!(r.pago, pago);
        assert_eq!(r.vuelto, a0 + a1 - pago);
    }
}

#[test]
fn pago_mayor_que_los_billetes_se_rechaza() {
    let _g = LOCK.lock().unwrap();
    assert_eq!(demo(100, 50, 151), Err(-1));
}
