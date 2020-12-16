module("luci.controller.watson_controller", package.seeall)

function index()
	entry({"admin", "services", "watson_model"}, cbi("watson_model"), _("IBM Watson IoT platform"),105)
end
