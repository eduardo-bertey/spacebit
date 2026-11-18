//! Enlace Rust al esquema Plus (pagos comunes, sin auditoría).
//!
//! `build.rs` compila los fuentes C de `plus/` y el shim; el binario Rust
//! llama a una ronda completa con montos elegidos.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct DemoPlus {
    pub verify_ok: u64,
    pub verify_sn_cero: u64,
    pub verify_falso: u64,
    pub l: u64,
    pub pago: u64,
    pub vuelto: u64,
    /// ciclos CPU: [setup, firmar, verificar]
    pub ciclos: [u64; 3],
}

unsafe extern "C" {
    fn matrict_plus_demo(
        amt0: u64,
        amt1: u64,
        pago: u64,
        v_ok: *mut u64,
        v_sn_cero: *mut u64,
        v_falso: *mut u64,
        l: *mut u64,
        pago_out: *mut u64,
        vuelto_out: *mut u64,
        ciclos: *mut u64,
    ) -> i32;
}

/// Devuelve `Err(-1)` si el pago supera `amt0 + amt1`.
pub fn demo(amt0: u64, amt1: u64, pago: u64) -> Result<DemoPlus, i32> {
    let mut r = DemoPlus {
        verify_ok: 0,
        verify_sn_cero: 0,
        verify_falso: 0,
        l: 0,
        pago: 0,
        vuelto: 0,
        ciclos: [0; 3],
    };
    let rc = unsafe {
        matrict_plus_demo(
            amt0,
            amt1,
            pago,
            &mut r.verify_ok,
            &mut r.verify_sn_cero,
            &mut r.verify_falso,
            &mut r.l,
            &mut r.pago,
            &mut r.vuelto,
            r.ciclos.as_mut_ptr(),
        )
    };
    if rc == 0 {
        Ok(r)
    } else {
        Err(rc)
    }
}
