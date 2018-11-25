extern crate treebitmap;
extern crate libc;

use libc::uint32_t;
use std::net::Ipv4Addr;

use treebitmap::*;

type RoutingTable = IpLookupTable<Ipv4Addr, u32>;

#[cfg(feature = "verbose")]
macro_rules! log {
    ($($x:expr),*) => {
        println!($($x),*);
    };
}

#[cfg(not(feature = "verbose"))]
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
pub extern fn rt_match(ptr: *mut RoutingTable, ip: uint32_t, prefix: uint32_t, exact: uint32_t) -> uint32_t {
    let _tb = unsafe { &mut *ptr };
    let _ip = Ipv4Addr::from(ip);
    
    let result = match exact {
        0 => match _tb.longest_match(_ip) {
            Some((_, _, index)) => index.clone(),
            None => 0
        }
        _ => match _tb.exact_match(_ip, prefix) {
            Some(index) => index.clone(),
            None => 0 // 0 means not found
        }
    };

    log!("Match: {}/{} via {} mode {}", _ip, prefix, result, match exact {
        0 => "longest",
        _ => "exact"
    });
    result
}

#[no_mangle]
pub extern fn rt_lookup(ptr: *mut RoutingTable, ip: uint32_t) -> uint32_t {
    rt_match(ptr, ip, 0, 0)
} 

#[no_mangle]
pub extern fn rt_cleanup(ptr: *mut RoutingTable) {
    log!("{}", "Cleanup");
    let _tb: Box<RoutingTable> = unsafe{ std::mem::transmute(ptr) };
    // drop this object
}

#[no_mangle]
pub extern fn rt_iterate(ptr: *mut RoutingTable, next_of: uint32_t) -> uint32_t {
    let _tb = unsafe { &mut *ptr };

    let mut iter = _tb.iter();

    if next_of != 0 {
        loop {
            match iter.next() {
                Some((_, _, index)) => {
                    if index == &next_of {
                        break;
                    }
                },
                None => { break }
            }
        }
    }
    
    let index = match iter.next() {
        Some((_, _, index)) => index.clone() as i32,
        None => -1, // -1 means end of iteration
    };

    log!("Iterate: from {}, get {}", next_of, index);

    index as uint32_t
}