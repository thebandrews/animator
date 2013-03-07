#ifndef PROPERTIES_H
#define PROPERTIES_H
/**
 * A Property object represents an aspect of a model that Modeler (or another
 * program) can modify.  This file contains several Property classes that
 * represent different kinds of properties.
 *
 * @author Jeff Booth (boothjmx@cs)
 */
#include <vector>

/** This needs to be defined somewhere.  TODO: put it in a better place? */
const double M_PI = 3.141592653589793;

// When the user removes keyframes, all points within REMOVE_KEYFRAME_RANGE
// are deleted.
const float REMOVE_KEYFRAME_RANGE = 0.2f;

class Property; // forward-declare Property because PropertyList is used
                // in class Property.
/**
 * This type represents lists of Property objects.
 * DO NOT USE IT for grouping your Modeler controls; use GroupProperty instead.
 */
typedef std::vector<Property*> PropertyList;

/**
 * A SignalDispatcher calls callbacks when events occur.  Callbacks accept
 * three parameters: a pointer to the object that dispatched the signal,
 * a user data parameter and a signal parameter.
 *
 * The Property class extends SignalDispatcher so it can notify sliders
 * whenever the property's value is changed.
 */
class SignalDispatcher;
typedef void (*SignalListener)(SignalDispatcher* dispatcher, void*, void*);
typedef std::pair<SignalListener, void*> SignalPair;
typedef std::vector<SignalPair> SignalListenerList;
class SignalDispatcher {
protected:
	SignalListenerList listeners;
public:
	/** Call all registered callbacks, passing data as the third parameter. */
	void signal(void* data = NULL);

	/** Register the given callback with the given data parameter. */
	void listen(SignalListener listener, void* data = NULL);

	/** Remove all callbacks from the callback list. */
	void clear();

	/** Remove a particular callback. */
	void unlisten(SignalListener listener);

	/** Virtual destructor to make sure the subclass's destructor is called. */
	virtual ~SignalDispatcher();
};

/**
 * Base class for properties.
 */
class Property : public SignalDispatcher {
protected:
	const char* name;
	int curveIndex;

public:
	/**
	 * Creates a new Property with a name.
	 */
	Property(const char* name);

	/**
	 * Returns the property's name
	 */
	const char* getName() const;

	/**
	 * Sets the property's name
	 */
	void setName(const char* name);

	/**
	 * Returns a list of child properties, or NULL if this property has no
	 * children.
	 */
	virtual PropertyList* getProperties();

	/**
	 * Obtains the property's curve index in the graph widget.
	 * TODO: we need a better definition for curve classes...
	 */
	int getCurveIndex();

	/** Sets the curve index. */
	void setCurveIndex(int index);
};

/**
 * A RangeProperty has a float value which must fall within a given range.
 */
class RangeProperty : public Property {
protected:
	float value, min, max, step;

public:
	/**
	 * Creates a new RangeProperty with the given name, minimum value,
	 * and maximum value.
	 */
	RangeProperty(
		const char* name,
		const float min,
		const float max,
		const float value,
		const float step);

	const float getValue() const;
	const float getMin() const;
	const float getMax() const;
	const float getStep() const;

	void setValue(const float value);
	void setMin(const float min);
	void setMax(const float max);
	void setStep(const float step);
};

/**
 * Lets you group properties together under one name.
 */
class GroupProperty : public Property {
protected:
	/** List of properties in the group. */
	PropertyList properties;

	/** True if the group appears collapsed by default in a tree view. */
	bool collapsed;

public:
	GroupProperty(const char* name, bool collapsed = false);
	PropertyList* getProperties();
	bool getCollapsed() const;
	void setCollapsed(bool c);

	/**
	 * Call this method to add properties to your group.
	 * This method returns the GroupProperty object you called it on, so
	 * you can "chain" your adds using a "fluent" interface:
	 *
	 *   groupProperty.add(&property1).add(&property2).add(&property3);
	 */
	GroupProperty& add(Property* prop);
};

/**
 * This special property contains three color components, and will appear
 * as a color chooser in the interface (even though it works just like a
 * GroupProperty).
 */
class RGBProperty : public Property {
protected:
	GroupProperty properties;

	// A RangeProperty for each color component.
	RangeProperty red, green, blue;
public:
	/**
	 * Creates a new RGBProperty.
	 *
	 * @param name  The name of the property
	 * @param r     Initial red value, between 0.0 and 1.0 inclusive
	 * @param g     Initial green value, between 0.0 and 1.0 inclusive
	 * @param b     Initial blue value, between 0.0 and 1.0 inclusive
	 */
	RGBProperty(const char* name, float r = 1, float g = 1, float b = 1);

	/** Returns the range properties. */
	PropertyList* getProperties();

	/** Accessors */
	float getRed() const;
	float getGreen() const;
	float getBlue() const;

	/** Mutators */
	void setRed(const float r);
	void setGreen(const float g);
	void setBlue(const float b);
	void setColor(const float r, const float g, const float b);
};

/**
 * Unlike the RangeProperty, which can have any value inside a continuous
 * range, the DiscreteProperty can only have an integer value.  You use this
 * control like a "switch" rather than a "slider": 
 */

/**
 * This property can be either true or false, and appears as a button on the
 * control list.  It's useful for hiding or showing things in your model.
 */
class BooleanProperty : public Property {
protected:
	bool val;

public:
	/**
	 * Creates a new BooleanProperty.
	 *
	 * @param name  Name of the property.
	 * @param value Property's value.
	 */
	BooleanProperty(const char* name, bool value = false);

	/** Returns the property's value. */
	bool getValue() const;

	/** Sets the property's value to v. */
	void setValue(const bool v);
};

/**
 * This property can be one of a list of choices.
 */
class ChoiceProperty : public Property {
protected:
	int val;
	const char* labels;

public:
	/**
	 * Creates a new ChoiceProperty.
	 * Example:
	 *     ChoiceProperty("name", "choice 0|choice 1|choice 2", 0);
	 *
	 * @param name    Name of the property
	 * @param labels  String labels for each choice, separated by | characters.
	 * @param value   Property's initial value (first choice is 0)
	 */
	ChoiceProperty(const char* name, const char* labels, const int value);

	/** Returns the property's value. */
	int getValue() const;

	/** Sets the property's value to v. */
	void setValue(const int v);

	/** Returns the string containing the choice labels. */
	const char* getLabels() const;
};

#endif // PROPERTIES_H