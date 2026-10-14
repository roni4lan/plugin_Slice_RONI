#pragma once
#include "../model/Composition.h"
#include "../modulation/AudioAnalyzer.h"
#include "../modulation/BeatClock.h"
#include "../modulation/ModulationMatrix.h"
#include "../control/OscServer.h"
namespace sb {void drawControlPanels(Project&,const Composition&,const std::string& selected,AudioAnalyzer&,BeatClock&,ModulationMatrix&,OscServer&);}
