map = Map("watson", "IBM Watson IoT platform")

section = map:section(NamedSection, "watson_sct", "watson", "Configuration")

flag = section:option(Flag, "enable", "Enable", "Connect to IBM Watson IoT platform")

orgId = section:option(Value, "orgId", "Organisation ID")
deviceType = section:option(Value, "deviceType", "Device type")
deviceId = section:option(Value, "deviceId", "Device ID")

token = section:option(Value, "token", "Token / Password")
token.datatype = "credentials_validate"
token.password = true
token.placeholder = "Password"
token.rmempty = false

return map
