function format_mem_size(s) {
    var mo_bytes = 1024 * 1024;
    var go_bytes = 1024 * mo_bytes;

    var o = s % 1024;
    var k = Math.round((s / 1024) % 1024);
    var m = Math.round((s / (1024*1024)) % 1024);
    var g = Math.round(s / (1024*1024*1024));

    if (g >= 1)
        return g+"Go ";

    if (m >= 1)
        return m+"Mo";

    if (k >= 1)
        return k+"Ko";
    
    if (o > 0)
        return o+"o";

    return "0";
};

function type_to_str(stype) {
    var stype_str = "N/A";

    if (stype & 0x0100) {
        stype_str = "Sensor";
    } else if (stype & 0x0200) {
        stype_str = "NVidia";
    } else if (stype & 0x0400) {
        stype_str = "HDD";
    } else if (stype & 0x1000) {
        stype_str = "AMD";
    }

   if (stype & 0x0001) {
       stype_str += " Temperature";
   } else if (stype & 0x0002) {
       stype_str += " Fan";
   }

    return stype_str;
};

function type_to_unit(stype) {
    if (stype & 0x0001) {
        unit = " C";
    } else if (stype & 0x0002) {
        unit = " RPM";
    }

    return unit;
}
