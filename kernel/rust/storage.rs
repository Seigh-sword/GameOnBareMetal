#![no_std]

// Decode a little-endian on-disk word. This is used by the GmStg driver through
// a C ABI; keeping bytes as values avoids unaligned pointer access.
#[no_mangle]
pub extern "C" fn gmstg_rust_u32(a: u8, b: u8, c: u8, d: u8) -> u32 {
    u32::from_le_bytes([a, b, c, d])
}
