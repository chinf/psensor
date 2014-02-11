/*
 * Copyright (C) 2010-2011 jeanfi@gmail.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

function format_mem_size(s) {
    var mo_bytes, go_bytes, o, k, m, g;

    mo_bytes = 1024 * 1024;
    go_bytes = 1024 * mo_bytes;

    o = s % 1024;
    k = Math.round((s / 1024) % 1024);
    m = Math.round((s / (1024*1024)) % 1024);
    g = Math.round(s / (1024*1024*1024));

    if (g >= 1)
        return g+"Go ";

    if (m >= 1)
        return m+"Mo";

    if (k >= 1)
        return k+"Ko";
    
    if (o > 0)
        return o+"o";

    return "0";
}

function type_to_str(stype) {
    var stype_str;

    if (stype & 0x0200) 
        stype_str = "NVidia ";
    else if (stype & 0x0800)
        stype_str = "ATI/AMD ";
    else 
	stype_str = "";
 
    if (stype & 0x04000)
	stype_str += "HDD ";
    else if (stype & 0x08000)
	stype_str += "CPU ";
    else if (stype & 0x10000)
	stype_str += "GPU ";
    else if (stype & 0x20000)
	stype_str += "Fan ";

    if (stype & 0x0001)
       stype_str += "Temperature";
    else if (stype & 0x0002)
       stype_str += "RPM";
    else if (stype & 0x0004)
       stype_str += "Load";

    return stype_str;
}

function type_to_unit(stype) {
    if (stype & 0x0001)
        unit = "C";
    else if (stype & 0x0002)
        unit = "RPM";

    return unit;
}

function value_to_str(value, type) {
    return value+type_to_unit(type);
}

function get_url_params() {
    var vars, hashes, i;

    vars = [];
    hashes = window.location.href.slice(window.location.href.indexOf('?') + 1).split('&');

    for(i = 0; i < hashes.length; i++) {
        hash = hashes[i].split('=');
        vars.push(hash[0]);
        vars[hash[0]] = hash[1];
    }

    return vars;
}

function update_chart(chart_id, title, data) {
    var min_date, max_date, min, max, value;
    var measures, data_chart, date, entry;
    var style;

    $("#"+chart_id).html("");

    measures = data["measures"];
    data_chart = [];
    
    $("h1").html("");
    $("h1").append(data["name"]);

    try {
	$("title").html(data["name"]);
    } catch(ignore) {
	// IE8 doesn't allow to modify the page title
    }
    
    $.each(measures, function(i, item) {
        value = item["value"];
        date = new Date(item["time"]*1000);
	entry = [date, item["value"]];
	
        data_chart.push(entry);
	
      	if (!max_date || max_date < date)
            max_date = date;        
	if (!min_date || min_date > date)
	    min_date = date;
        
        if (!min || value < min)
            min = value;
	if (!max || value > max)
            max = value;	
    });
    
    style = { 
	title: title,
	axes: { 
	    xaxis: {
		renderer: $.jqplot.DateAxisRenderer,
		tickOptions: {
		    formatString:'%H:%M:%S'
		},
        	min: min_date,
                max: max_date
	    },
	    yaxis: {
		min: min-1,
		max: max+1
	    }
	},
	series: [ { 
	    lineWidth: 1, 
	    showMarker: false 
	} ]
    };


    $.jqplot (chart_id, [data_chart], style);
}

function update_menu() {
    var name, link, url, str;

    str = "";

    $.getJSON("/api/1.1/sensors", function(data) {
	str += "<li><a href=\"index.html#cpu\">CPU</a><ul>";
	url = "details.html?id="+escape("/api/1.1/cpu/usage");
	link = "<a href='"+url+"'>usage</a>";
	str += "<li>"+link+"</li>";
	str += "</li></ul>";	

	str += "<li><a href=\"index.html#network\">Network</a></li>";

	str += "<li><a href=\"index.html#memory\">Memory</a></li>";

	str += "<li><a href=\"index.html#sensors\">Sensors</a>\n<ul>";
	$.each(data, function(i, item) {
            name = item["name"];
	    url = "details.html?id="+escape("/api/1.1/sensors/"+item["id"]);
	    link = "<a href='"+url+"'>"+name+"</a>";
	    str += "<li>"+link+"</li>";
	});
	str += "</li></ul>";
	
	$("#menu-list").append(str);

    });

}

function update_summary_sensors() {
    var name, value_str, min_str, max_str, type, type_str, url;

    $.getJSON("/api/1.1/sensors", function(data) {
	$("#sensors tbody").html("");

        $.each(data, function(i, item) {	    
            name = item["name"];
            type = item["type"];
            value_str = value_to_str(item["last_measure"]["value"], type);
            min_str = value_to_str(item["min"], type);
            max_str = value_to_str(item["max"], type);
	    type_str = type_to_str(type);
	    url = "details.html?id="+escape("/api/1.1/sensors/"+item["id"]);

            $("#sensors tbody").append("<tr>"
	                         +"<td><a href='"+url+"'>"+name+"</a></td>"
	                         +"<td>"+value_str+"</td>"
				 +"<td>"+min_str+"</td>"
				 +"<td>"+max_str+"</td>"
				 +"<td>"+type_str+"</td>"
				 +"</tr>");                 
        });          
    });
}

function update_summary_sysinfo() {
    $.getJSON("/api/1.1/sysinfo", function(data) {
	$("#uptime").html("");
	$("#cpu tbody").html("");
	$("#memory").html("");
	$("#swap").html("");
	$("#net tbody").html("");

        var load = Math.round(data["load"] * 100);
        var load_1 = Math.round(data["load_1"]*1000)/1000;
        var load_5 = Math.round(data["load_5"]*1000)/1000;
        var load_15 = Math.round(data["load_15"]*1000)/1000;
        var uptime = data["uptime"];
        var uptime_s = uptime % 60;
        var uptime_mn = Math.floor((uptime / 60) % 60);
        var uptime_h = Math.floor((uptime / (60*60)) % 24);
        var uptime_d = Math.floor(uptime / (60*60*24));
	
        $("#cpu").append("<tr><td><a href='details.html?id=/api/1.1/cpu/usage'>"+load+"%</a></td><td>"
			 +load_1+"</td><td>"
			 +load_5+"</td><td>"
			 +load_15+"</td></tr>");
	
        $("#uptime").append(uptime_d+"d "+uptime_h+"h "+uptime_mn+"mn");
	
        var ram = data["ram"];
        var swap = data["swap"];
        var mu = data["mem_unit"];
	
	var ramtotal = ram["total"]*mu;
        var ramfree = ram["free"]*mu;
        var ramused = (ram["total"] - ram["free"])*mu;
        var ramshared = ram["shared"]*mu;
        var rambuffer = ram["buffer"]*mu;
        
	
        $("#memory").append("<td>Memory</td>"
			    +"<td>"+format_mem_size(ramtotal)+"</td>"
			    +"<td>"+format_mem_size(ramused)+"</td>"
			    +"<td>"+format_mem_size(ramfree)+"</td>"
			    +"<td>"+format_mem_size(ramshared)+"</td>"
			    +"<td>"+format_mem_size(rambuffer)+"</td>");
	
        $("#swap").append("<td>Swap</td>"
			  +"<td>"+format_mem_size(swap["total"]*mu)+"</td>"
			  +"<td>"+format_mem_size(swap["total"]*mu-swap["free"]*mu)+"</td>"
			  +"<td>"+format_mem_size(swap["free"]*mu)+"</td>");
	
        var netdata = data["net"];
        $.each(netdata, function(i, item) {
            $("#net").append("<tr><td>"+item["name"]+"</td>"
                             +"<td>"+format_mem_size(item["bytes_in"])+"</td>"
                             +"<td>"+format_mem_size(item["bytes_out"])+"</td></tr>");
        });
    });
}