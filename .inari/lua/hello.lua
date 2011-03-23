--[[
-- This is a simple lua plugin designed to test out
-- inari's Lua/C bridge
--]]

PluginName = "lua"
AdminOnly  = false

function plugin_init ()
  -- nothing to be done for init
end

function plugin (irc)
  irc:privmsg(irc.chan, 2, irc.nick .. ": Hello from lua!")
end
