local ConfigService = require("api/ConfigService")

local TuyaDaemon = ConfigService:new({})

local TuyaConfig = TuyaDaemon:section(
	"tuya-daemon",
	"tuya_daemon"
)

TuyaConfig:make_primary()

local enabled = TuyaConfig:option("enabled")
function enabled:validate(value)
	return self.dt:is_bool(value)
end

local device_id = TuyaConfig:option("device_id")
device_id.maxlength = 256

local device_secret = TuyaConfig:option("device_secret")
device_secret.maxlength = 256

local product_id = TuyaConfig:option("product_id")
product_id.maxlength = 256
--------------------------------------------------
DAEMON CONTROL ACTIONS
--------------------------------------------------

function TuyaDaemon:StartAction()
	local result = os.execute("/etc/init.d/tuya-daemon start")

	if result == 0 or result == true then
		return self:ResponseOK({
			result = "Tuya daemon started"
		})
	end

	self:add_error(500, "Failed to start Tuya daemon", "tuya_daemon")
	return self:ResponseOK()
end


function TuyaDaemon:StopAction()
	local result = os.execute("/etc/init.d/tuya-daemon stop")

	if result == 0 or result == true then
		return self:ResponseOK({
			result = "Tuya daemon stopped"
		})
	end

	self:add_error(500, "Failed to stop Tuya daemon", "tuya_daemon")
	return self:ResponseOK()
end


function TuyaDaemon:RestartAction()
	local result = os.execute("/etc/init.d/tuya-daemon restart")

	if result == 0 or result == true then
		return self:ResponseOK({
			result = "Tuya daemon restarted"
		})
	end

	self:add_error(500, "Failed to restart Tuya daemon", "tuya_daemon")
	return self:ResponseOK()
end


function TuyaDaemon:ReportNowAction()
	local handle = io.popen("ubus call tuya_daemon report_now 2>&1")
	local output = handle:read("*a")
	handle:close()

	return self:ResponseOK({
		result = output
	})
end


function TuyaDaemon:StatusAction()
	local handle = io.popen("ubus call tuya_daemon status 2>/dev/null")
	local output = handle:read("*a")
	handle:close()

	return self:ResponseOK({
		result = output
	})
end


local start_action =
	TuyaDaemon:action("start", TuyaDaemon.StartAction)

local stop_action =
	TuyaDaemon:action("stop", TuyaDaemon.StopAction)

local restart_action =
	TuyaDaemon:action("restart", TuyaDaemon.RestartAction)

local report_action =
	TuyaDaemon:action("report_now", TuyaDaemon.ReportNowAction)

local status_action =
	TuyaDaemon:action("status", TuyaDaemon.StatusAction)

return TuyaDaemon