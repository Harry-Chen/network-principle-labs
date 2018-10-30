extern crate treebitmap;
extern crate libc;

use libc::uint32_t;
use std::net::Ipv4Addr;

use treebitmap::*;

type RoutingTable = IpLookupTable<Ipv4Addr, u32>;

#[cfg(not(feature = "release"))]
macro_rules! log {
    ($($x:expr),*) => {
        println!($($x),*);
    };
}

#[cfg(feature = "release")]
macro_rules! log {
    ($($x:expr),*) => { };
}

#[no_mangle]
pub extern fn rt_init() -> *mut RoutingTable {
    log!("{}", "Init");
    let _ptr = unsafe { std::mem::transmute(Box::new(RoutingTable::new())) };
    _ptr
}

#[no_mangle]
pub extern fn rt_insert(ptr: *mut RoutingTable, ip: uint32_t, prefix: uint32_t, index: uint32_t) {
    let _tb = unsafe { &mut *ptr };
    let _ip = Ipv4Addr::from(ip);
    log!("Insert: {}/{} via {}", _ip, prefix, index);
    _tb.insert(_ip, prefix, index);
}

#[no_mangle]
pub extern fn rt_remove(ptr: *mut RoutingTable, ip: uint32_t, prefix: uint32_t) {
    let _tb = unsafe { &mut *ptr };
    let _ip = Ipv4Addr::from(ip);
    log!("Remove: {}/{}", _ip, prefix);
    _tb.remove(_ip, prefix);
}

#[no_mangle]
pub extern fn rt_lookup(ptr: *mut RoutingTable, ip: uint32_t) -> uint32_t {
    let _tb = unsafe { &mut *ptr };
    let _ip = Ipv4Addr::from(ip);
    let result = _tb.longest_match(_ip);
    let index = match result {
        Some((_, _, index)) => index.clone(),
        None => 0, // 0 means not found
    };
    log!("Lookup: {} via {}", _ip, index);
    index
}

#[no_mangle]
pub extern fn rt_cleanup(ptr: *mut RoutingTable) {
    log!("{}", "Cleanup");
    let _tb: Box<RoutingTable> = unsafe{ std::mem::transmute(ptr) };
    // drop this object
}