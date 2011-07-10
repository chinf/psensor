--
-- Convenient functions for HTML output
-- 

function td(content)
   return "<td>" .. content .. "</td>"
end

function th(style, content)
   if style then
      return "<th class='"..style.."'>"..content.."</th>"
   else
      return "<th>"..content.."</th>"
   end
end

function tr(style,...)
   if style then
      ret = "<tr class='"..style.."'>"
   else 
      ret = "<tr>"
   end

   for i,s in ipairs(arg) do
      ret = ret .. s
   end

   ret = ret .. "</tr>\n"

   return ret
end

function h2(str) 
   return "<h2>"..str.."</h2>\n"
end


-- Formats sensor information to HTML 'tr'
function sensor_to_tr(id,sensor)
   return tr(nil,
	     td(sensor["name"]),
	     td(sensor["measure_last"]),
	     td(sensor["measure_min"]),
	     td(sensor["measure_max"]))
end


-- Formats number of bytes to string
function format_mem_size(bytes)
   if (bytes == 0) then
      return "0"
   end

   if (bytes < 1024) then
      return bytes .. " o";
   end

   mo_bytes = 1024 * 1024;

   if (bytes < mo_bytes) then
      return math.ceil(bytes / 1024) .. " Ko"
   end

   go_bytes = 1024 * mo_bytes;

   if (bytes < go_bytes) then
      return math.ceil(bytes / mo_bytes) .. " Mo"
   end

   return math.ceil(bytes / go_bytes) .. " Go"
end

-- Formats uptime to string
function format_uptime(uptime)
   uptime_s = sysinfo["uptime"]%60
   uptime_mn = math.floor( (sysinfo["uptime"] / 60) % 60)
   uptime_h = math.floor( (sysinfo["uptime"] / (60*60)) % 24)
   uptime_d = math.floor(sysinfo["uptime"] / (60*60*24))

   return uptime_d .. "d " ..
      uptime_h .. "h " ..
      string.format("%02.d",uptime_mn) .. "mn " ..
      string.format("%02d",uptime_s) .. "s"
end

str = "<html><head><link rel='stylesheet' type='text/css' href='/style.css' /></head><body><h1>Psensor Monitoring Server</h1>"

if sysinfo then

--
-- Uptime
--

   str = str .. "<p><strong>Uptime</strong>: " .. format_uptime(sysinfo["uptime"]) .. "</p>"

--
-- CPU
--

   str = str .. h2("CPU")

   str = str .. "<table>"
      .. "<thead>" .. tr("title",
	    th(nil,"Current usage"),
	    th(nil,"Load 1mn"),
	    th(nil,"Load 5mn"),
	    th(nil,"Load 15mn")) .. "</thead>"

   str = str .. "<tbody><tr>"

   if sysinfo["load"] then
      str = str .. td(math.ceil(100*sysinfo["load"]) .. "%")
   else
      str = str .. td("N/A")
   end

   str = str  .. td(string.format("%.2f",sysinfo["load_1mn"])) ..
   td(string.format("%.2f",sysinfo["load_5mn"])) ..
   td(string.format("%.2f",sysinfo["load_15mn"])) ..
   "</tbody></tr>" ..
   "</table>"

--
-- Memory
--

   totalram = format_mem_size(sysinfo["totalram"] * sysinfo["mem_unit"])
   freeram = format_mem_size(sysinfo["freeram"] * sysinfo["mem_unit"])
   sharedram = format_mem_size(sysinfo["sharedram"] * sysinfo["mem_unit"])
   bufferram = format_mem_size(sysinfo["bufferram"] * sysinfo["mem_unit"])
   usedram = format_mem_size(sysinfo["totalram"] - sysinfo["freeram"])

   totalswap = format_mem_size(sysinfo["totalswap"] * sysinfo["mem_unit"])
   freeswap = format_mem_size(sysinfo["freeswap"] * sysinfo["mem_unit"])
   usedswap = format_mem_size(sysinfo["totalswap"] - sysinfo["freeswap"])

   str = str 
      .. h2("Memory")

      .. "<table>"

      .. tr("title",
	    th(nil,""),
	    th(nil,"Total"),
	    th(nil,"Used"),
	    th(nil,"Free"),
	    th(nil,"Shared"),
	    th(nil,"Buffer"))

      .. tr(nil,
	    th(nil,"Memory"),
	    td(totalram),
	    td(usedram),
	    td(freeram),
	    td(sharedram),
	    td(bufferram))
   
       .. tr(nil,
	     th(nil,"Swap"),
	     td(totalswap),
	     td(usedswap),
	     td(freeswap))
    
       .. "</table>"
				   
end

--
-- Sensors
-- 

if sensors then

   str = str .. "<h2>Sensors</h2>"
      .. "<table>"
      .. "<tr class='title'><th>Name</th><th>Value</th><th>Min</th><th>Max</th></tr>"

   for i,sensor in ipairs(sensors) do 
      str = str .. sensor_to_tr(i,sensor) 
   end
   
   str = str .. "</table><hr /><a href='http://wpitchoune.net/psensor'>psensor-server</a></body></html>"

end


return str

