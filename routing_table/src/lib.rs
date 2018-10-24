extern crate treebitmap;
extern crate libc;

use libc::uint32_t;
use std::net::{Ipv4Addr, Ipv6Addr};
use std::str::FromStr;
use treebitmap::*;

#[no_mangle]
pub extern fn addition(a: uint32_t, b: uint32_t) -> uint32_t {
    a + b
}

#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
    let mut tbm = IpLookupTable::new();
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
    }
}
