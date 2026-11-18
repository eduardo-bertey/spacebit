//! API segura sobre el shim C de MatRiCT-Au (v4).
//!
//! El C hace una ronda completa (keygen / mint / spend / verify / audit) y
//! devuelve los resultados. `build.rs` compila los fuentes automáticamente:
//! no hace falta generar `libmatrict_au.a` a mano.

use std::ffi::CString;
use std::os::raw::c_char;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ResultadoDemo {
    pub verify_ok: u64,
    pub verify_alterado: u64,
    pub l_real: u64,
    pub l_audit: u64,
    pub amt0_real: u64,
    pub amt0_audit: u64,
    pub amt1_real: u64,
    pub amt1_audit: u64,
}

impl ResultadoDemo {
    fn nuevo() -> Self {
        Self {
            verify_ok: 0,
            verify_alterado: 0,
            l_real: 0,
            l_audit: 0,
            amt0_real: 0,
            amt0_audit: 0,
            amt1_real: 0,
            amt1_audit: 0,
        }
    }

    /// El observador recuperó la misma cuenta y los mismos montos.
    pub fn audit_cuadra(&self) -> bool {
        self.l_real == self.l_audit
            && self.amt0_real == self.amt0_audit
            && self.amt1_real == self.amt1_audit
    }
}

unsafe extern "C" {
    fn matrict_demo_pago(
        texto: *const c_char,
        texto_alterado: *const c_char,
        amt_in: u64,
        amt_pago: u64,
        verify_ok: *mut u64,
        verify_alterado: *mut u64,
        l_real: *mut u64,
        l_audit: *mut u64,
        amt0_real: *mut u64,
        amt0_audit: *mut u64,
        amt1_real: *mut u64,
        amt1_audit: *mut u64,
    ) -> i32;

    fn matrict_test_serial_cero(v_ok: *mut u64, v_cero: *mut u64) -> i32;
    fn matrict_test_firma_falsa(v_ok: *mut u64, v_falso: *mut u64) -> i32;
    fn matrict_test_fuera_rango(v_ok: *mut u64, v_falso: *mut u64) -> i32;
}

/// Ronda con montos elegidos por el llamador y texto firmado dentro del hash.
/// Devuelve `Err(-1)` si `amt_pago > amt_in`.
pub fn demo_pago(texto: &str, alterado: &str, amt_in: u64, amt_pago: u64) -> Result<ResultadoDemo, i32> {
    let texto = CString::new(texto).map_err(|_| -2)?;
    let alterado = CString::new(alterado).map_err(|_| -2)?;
    let mut r = ResultadoDemo::nuevo();

    let rc = unsafe {
        matrict_demo_pago(
            texto.as_ptr(),
            alterado.as_ptr(),
            amt_in,
            amt_pago,
            &mut r.verify_ok,
            &mut r.verify_alterado,
            &mut r.l_real,
            &mut r.l_audit,
            &mut r.amt0_real,
            &mut r.amt0_audit,
            &mut r.amt1_real,
            &mut r.amt1_audit,
        )
    };
    if rc != 0 {
        Err(rc)
    } else {
        Ok(r)
    }
}

/// Verifica `(normal, con_serial_cero)`. El parche debe dar `v_cero == 0`.
pub fn prueba_serial_cero() -> (u64, u64) {
    let (mut ok, mut cero) = (0, 0);
    let rc = unsafe { matrict_test_serial_cero(&mut ok, &mut cero) };
    debug_assert_eq!(rc, 0);
    (ok, cero)
}

/// Verifica `(normal, con_la_prueba_alterada)`.
pub fn prueba_firma_falsa() -> (u64, u64) {
    let (mut ok, mut falso) = (0, 0);
    let rc = unsafe { matrict_test_firma_falsa(&mut ok, &mut falso) };
    debug_assert_eq!(rc, 0);
    (ok, falso)
}

/// Verifica `(normal, con_el_firmante_fuera_del_anillo)`.
pub fn prueba_fuera_rango() -> (u64, u64) {
    let (mut ok, mut falso) = (0, 0);
    let rc = unsafe { matrict_test_fuera_rango(&mut ok, &mut falso) };
    debug_assert_eq!(rc, 0);
    (ok, falso)
}
