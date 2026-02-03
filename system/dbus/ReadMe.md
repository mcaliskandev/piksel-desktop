# DBus (The communication layer)

A message bus so PikselDesktop and PikselSystem can talk without being in the same process.

Shell will ask these questions to System via DBUS ;  
“What’s the current theme?”  
“Update wallpaper.”  
“Change panel position.”  
“Save this setting.”  

it is shortly ; The API / bridge between UI and system logic.  

**TODO :**  
A simple DBus service: org.piksel.System  
A method: GetSetting(key)  
A method: SetSetting(key, value)  
A signal: ThemeChanged  
