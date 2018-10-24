extern crate treebitmap;
extern crate libc;

use libc::uint32_t;
use std::net::Ipv4Addr;
use std::str::FromStr;

use treebitmap::*;

type RoutingTable = IpLookupTable<Ipv4Addr, u32>;

#[no_mangle]
pub extern fn rt_init() -> *mut RoutingTable {
    let _ptr = unsafe { std::mem::transmute(Box::new(RoutingTable::new())) };
    _ptr
}

#[no_mangle]
pub extern fn rt_test(ptr: *mut RoutingTable) -> bool {
    let mut tbm = unsafe { &mut *ptr };
    tbm.insert(Ipv4Addr::new(10, 0, 0, 0), 8, 100002);
    tbm.insert(Ipv4Addr::new(100, 64, 0, 0), 24, 10064024);
    tbm.insert(Ipv4Addr::new(100, 64, 1, 0), 24, 10064124);
    tbm.insert(Ipv4Addr::new(100, 64, 0, 0), 10, 100004);

    let result = tbm.longest_match(Ipv4Addr::new(10, 10, 10, 10));
    assert_eq!(result, Some((Ipv4Addr::new(10, 0, 0, 0), 8, &100002)));

    let result = tbm.longest_match(Ipv4Addr::new(100, 100, 100, 100));
    assert_eq!(result, Some((Ipv4Addr::new(100, 64, 0, 0), 10, &100004)));

    let result = tbm.longest_match(Ipv4Addr::new(100, 64, 0, 100));
    assert_eq!(result, Some((Ipv4Addr::new(100, 64, 0, 0), 24, &10064024)));

    let result = tbm.longest_match(Ipv4Addr::new(200, 200, 200, 200));
    assert_eq!(result, None);
    true
}
