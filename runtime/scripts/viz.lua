local viz = {}
local gfx = require 'gfx'
local ffi = require 'ffi'

local C = ffi.C
local CurrentViz
local wndWidth = 0
local wndHeight = 0

-- How to do locking?
function viz.texture2D(viz, panel)
  if viz.hnd then
    C.gfxBlitTexture(viz.hnd, panel.x, panel.y, panel.w, panel.h, wndWidth, wndHeight);
  end
end

function viz.set(vizdesc)
  CurrentViz = vizdesc
end

local vizPanelz = {}

function viz.showVisualisations()
  wndWidth  = g_windowWidth;
  wndHeight = g_windowHeight;

  vizPanelz[1] = {
    x = wndWidth - 300,
    y = wndHeight - 300,
    w = 300,
    h = 300,
  }

  if CurrentViz and CurrentViz.viz then
    for idx, panel in ipairs(vizPanelz) do
      C.uiVisualiserFrame(panel.x, panel.y, panel.w, panel.h)
      CurrentViz.viz(CurrentViz, panel)
    end
  end
end

return viz
