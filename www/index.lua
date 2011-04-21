
function footer()
   return "<div class='footer'>"
      .. "<a href='http://wpitchoune.net/psensor'>psensor-server</a> v"
      .. psensor_version 
      .. "</div>"
end

str = "\
<html>\
  <head>\
    <title>Psensor Web Server</title>\
    <link rel='stylesheet' type='text/css' href='/style.css' />\
  </head>\
\
  <body>\
\
  <h1>Welcome to the Psensor Web Server!</h1>\
\
  <p>Go to the <a href='monitor.lua'>Monitoring Page</a>.</p>\
\
  <hr />"
   .. footer() 
   .. "</body>\
\
</html>"

return str