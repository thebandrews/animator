#include "properties.h"

using namespace std;

////////// class SignalDispatcher //////////
void SignalDispatcher::clear() { listeners.clear(); }
void SignalDispatcher::listen(SignalListener listener, void* data) {
	listeners.push_back(SignalPair(listener, data));
}
void SignalDispatcher::signal(void* data) {
	for (SignalListenerList::iterator iter = listeners.begin();
		iter != listeners.end();
		iter++) {
		iter->first(this, iter->second, data);
	}
}
void SignalDispatcher::unlisten(SignalListener listener) {
	for (SignalListenerList::iterator iter = listeners.begin();
		iter != listeners.end();
		iter++) {
		if (iter->first == listener) {
			listeners.erase(iter);
			break;
		}
	}
}
SignalDispatcher::~SignalDispatcher() {}

////////// class Property //////////
Property::Property(const char* name) {
	this->name = name; this->curveIndex = -1;
}
const char* Property::getName() const {	return name; }
void Property::setName(const char* name) { this->name = name; signal(); }
PropertyList* Property::getProperties() { return NULL; }
int Property::getCurveIndex() { return curveIndex; }
void Property::setCurveIndex(int index) { curveIndex = index; }

////////// class RangeProperty //////////
RangeProperty::RangeProperty(
	const char* name,
	const float min,
	const float max,
	const float value,
	const float step) : Property(name) {
	this->min = min; this->max = max; this->value = value; this->step = step;
}
const float RangeProperty::getMin()   const { return min; }
const float RangeProperty::getMax()   const { return max; }
const float RangeProperty::getValue() const { return value; }
const float RangeProperty::getStep()  const { return step; }
void RangeProperty::setMin(const float min) { this->min = min; signal(); }
void RangeProperty::setMax(const float max) { this->max = max; signal(); }
void RangeProperty::setValue(const float value) { this->value = value; signal(); }
void RangeProperty::setStep(const float step) { this->step = step; signal(); }

////////// class GroupProperty //////////
GroupProperty::GroupProperty(const char* name, bool collapsed) :
	Property(name), collapsed(collapsed) {}
GroupProperty& GroupProperty::add(Property *prop) {
	properties.push_back(prop);
	return *this;
}
bool GroupProperty::getCollapsed() const { return collapsed; }
void GroupProperty::setCollapsed(const bool c) { collapsed = c; signal(); }
PropertyList* GroupProperty::getProperties() { return &properties; }

////////// class RGBProperty //////////
RGBProperty::RGBProperty(const char* name, float r, float g, float b) :
	Property(name),
	red("Red", 0, 1, r, .1f),
	green("Green", 0, 1, g, .1f),
	blue("Blue", 0, 1, b, .1f),
	properties(name) {
	properties.add(&red).add(&green).add(&blue);
}
PropertyList* RGBProperty::getProperties() {
	return properties.getProperties();
}
float RGBProperty::getRed() const { return red.getValue(); }
float RGBProperty::getGreen() const { return green.getValue(); }
float RGBProperty::getBlue() const { return blue.getValue(); }
void RGBProperty::setRed(const float r) { red.setValue(r); signal(); }
void RGBProperty::setGreen(const float g) { green.setValue(g); signal(); }
void RGBProperty::setBlue(const float b) { blue.setValue(b); signal(); }
void RGBProperty::setColor(const float r, const float g, const float b) {
	red.setValue(r); green.setValue(g); blue.setValue(b); signal();
}

////////// class BooleanProperty //////////
BooleanProperty::BooleanProperty(const char *name, bool value) :
	Property(name) {
	val = value;
}
bool BooleanProperty::getValue() const { return val; }
void BooleanProperty::setValue(const bool v) { val = v; signal(); }

////////// class ChoiceProperty ///////////
ChoiceProperty::ChoiceProperty(const char* name, const char* labels,
							   const int value) :
	Property(name) {
	val = value;
	this->labels = labels;
}
int ChoiceProperty::getValue() const { return val; }
void ChoiceProperty::setValue(const int v) { val = v; signal(); }
const char* ChoiceProperty::getLabels() const { return labels; }