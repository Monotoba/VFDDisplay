#include "VFDM202MD15HAL.h"
#include "../Capabilities/CapabilitiesRegistry.h"
#include <string.h>

VFDM202MD15HAL::VFDM202MD15HAL() {
    _capabilities = CapabilitiesRegistry::createVFDM202MD15Capabilities();
    CapabilitiesRegistry::getInstance().registerCapabilities(_capabilities);
}

bool VFDM202MD15HAL::init() { if(!_transport){ _lastError=VFDError::TransportFail; return false;} bool ok=_cmdInit(); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }
bool VFDM202MD15HAL::reset() { return init(); }

bool VFDM202MD15HAL::clear() { bool ok=_cmdClear(); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }
bool VFDM202MD15HAL::cursorHome() { bool ok=_cmdHome(); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }

bool VFDM202MD15HAL::setCursorPos(uint8_t row, uint8_t col) {
    if(!_capabilities){ _lastError=VFDError::InvalidArgs; return false; }
    if(row>=_capabilities->getTextRows() || col>=_capabilities->getTextColumns()){ _lastError=VFDError::InvalidArgs; return false; }
    bool ok=_posRowCol(row,col); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDM202MD15HAL::setCursorBlinkRate(uint8_t rate_ms) { bool ok=_displayControl(true,false,(rate_ms!=0)); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }

bool VFDM202MD15HAL::writeCharAt(uint8_t row, uint8_t column, char c){ return moveTo(row,column) && writeChar(c);} 
bool VFDM202MD15HAL::writeAt(uint8_t row, uint8_t column, const char* text){ return moveTo(row,column) && write(text);} 
bool VFDM202MD15HAL::moveTo(uint8_t row, uint8_t column){ return _posRowCol(row,column);} 

bool VFDM202MD15HAL::backSpace(){ return writeChar(0x08);} 
bool VFDM202MD15HAL::hTab(){ return writeChar(0x09);} 
bool VFDM202MD15HAL::lineFeed(){ return writeChar(0x0A);} 
bool VFDM202MD15HAL::carriageReturn(){ return writeChar(0x0D);} 

bool VFDM202MD15HAL::writeChar(char c){ if(!_transport) return false; return _writeData(reinterpret_cast<const uint8_t*>(&c),1);} 
bool VFDM202MD15HAL::write(const char* msg){ if(!_transport || !msg){ _lastError=VFDError::InvalidArgs; return false;} return _writeData(reinterpret_cast<const uint8_t*>(msg), strlen(msg)); }

bool VFDM202MD15HAL::centerText(const char* str, uint8_t row){ if(!_capabilities||!str){ _lastError=VFDError::InvalidArgs; return false;} uint8_t cols=_capabilities->getTextColumns(); size_t len=strlen(str); if(len>cols) len=cols; uint8_t pad=(uint8_t)((cols-len)/2); if(!setCursorPos(row,0)) return false; for(uint8_t i=0;i<pad;++i) if(!_writeData((const uint8_t*)" ",1)) return false; return write(str);} 

bool VFDM202MD15HAL::writeCustomChar(uint8_t index){ uint8_t code; if(!getCustomCharCode(index,code)){ _lastError=VFDError::InvalidArgs; return false;} return writeChar((char)code);} 
bool VFDM202MD15HAL::getCustomCharCode(uint8_t index, uint8_t& codeOut) const { if(!_capabilities) return false; if(index>=_capabilities->getMaxUserDefinedCharacters()) return false; codeOut=index; return true; }

bool VFDM202MD15HAL::setBrightness(uint8_t lumens){ uint8_t idx=(lumens<64)?3:(lumens<128)?2:(lumens<192)?1:0; bool ok=_functionSet(idx); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }
bool VFDM202MD15HAL::saveCustomChar(uint8_t index, const uint8_t* pattern){ return setCustomChar(index, pattern);} 
bool VFDM202MD15HAL::setCustomChar(uint8_t index, const uint8_t* pattern){ if(!_transport||!_capabilities||!pattern){ _lastError=VFDError::InvalidArgs; return false;} if(index>=8){ _lastError=VFDError::InvalidArgs; return false;} uint8_t addr=(uint8_t)((index & 0x07)*8); if(!_writeCmd((uint8_t)(0x40 | (addr&0x3F)))){ _lastError=VFDError::TransportFail; return false;} for(uint8_t r=0;r<8;++r){ uint8_t row=pattern[r] & 0x1F; if(!_writeData(&row,1)){ _lastError=VFDError::TransportFail; return false;} } _lastError=VFDError::Ok; return true; }

bool VFDM202MD15HAL::setDisplayMode(uint8_t mode){ (void)mode; _lastError=VFDError::NotSupported; return false; }
bool VFDM202MD15HAL::setDimming(uint8_t level){ uint8_t idx=(uint8_t)(level & 0x03); bool ok=_functionSet(idx); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }
bool VFDM202MD15HAL::cursorBlinkSpeed(uint8_t rate){ return setCursorBlinkRate(rate);} 
bool VFDM202MD15HAL::changeCharSet(uint8_t setId){ if(setId==0) return writeChar(0x18); if(setId==1) return writeChar(0x19); return false; }

bool VFDM202MD15HAL::sendEscapeSequence(const uint8_t* data){ (void)data; _lastError=VFDError::NotSupported; return false; }
bool VFDM202MD15HAL::hScroll(const char* str, int dir, uint8_t row){ (void)str;(void)dir;(void)row; _lastError=VFDError::NotSupported; return false; }
bool VFDM202MD15HAL::vScroll(const char* str, int dir){ (void)str;(void)dir; _lastError=VFDError::NotSupported; return false; }
bool VFDM202MD15HAL::vScrollText(const char* text, uint8_t startRow, ScrollDirection direction){ (void)text;(void)startRow;(void)direction; _lastError=VFDError::NotSupported; return false; }
bool VFDM202MD15HAL::starWarsScroll(const char* text, uint8_t startRow){ (void)text;(void)startRow; _lastError=VFDError::NotSupported; return false; }

bool VFDM202MD15HAL::flashText(const char* str, uint8_t row, uint8_t col, uint8_t on_ms, uint8_t off_ms){ (void)str;(void)row;(void)col;(void)on_ms;(void)off_ms; _lastError=VFDError::NotSupported; return false; }

int VFDM202MD15HAL::getCapabilities() const { return _capabilities?_capabilities->getAllCapabilities():0; }
const char* VFDM202MD15HAL::getDeviceName() const { return _capabilities?_capabilities->getDeviceName():"M202MD15"; }

// ===== NO_TOUCH primitives =====
bool VFDM202MD15HAL::_functionSet(uint8_t brightnessIndex){ uint8_t cmd=0x30; cmd|=0x08; cmd|=(brightnessIndex & 0x03); return _writeCmd(cmd);} 
bool VFDM202MD15HAL::_cmdInit(){ if(!_functionSet(0)) return false; if(!_displayControl(true,false,false)) return false; if(!_cmdClear()) return false; if(!_writeCmd(0x06)) return false; return true;} 
bool VFDM202MD15HAL::_cmdClear(){ return _writeCmd(0x01);} 
bool VFDM202MD15HAL::_cmdHome(){ return _writeCmd(0x02);} 
bool VFDM202MD15HAL::_posLinear(uint8_t addr){ return _writeCmd((uint8_t)(0x80 | (addr & 0x7F))); } 
bool VFDM202MD15HAL::_posRowCol(uint8_t row, uint8_t col){ uint8_t base[]={0x00,0x40}; if(row>=2) return false; return _posLinear((uint8_t)(base[row]+col)); } 
bool VFDM202MD15HAL::_displayControl(bool d,bool c,bool b){ uint8_t cmd=0x08 | (d?0x04:0) | (c?0x02:0) | (b?0x01:0); return _writeCmd(cmd);} 
bool VFDM202MD15HAL::_writeCmd(uint8_t cmd){ if(!_transport) return false; if(_transport->supportsControlLines()) (void)_transport->setControlLine("RS", false); return _transport->write(&cmd,1);} 
bool VFDM202MD15HAL::_writeData(const uint8_t* data, size_t len){ if(!_transport||!data||len==0) return false; if(_transport->supportsControlLines()) (void)_transport->setControlLine("RS", true); return _transport->write(data,len);} 

