#include"Events.h"

Events::Events(int x, int y, int z, int w) {
	type = x;
	id = y;
	extra = z;
	data = w;
}

EventLog::EventLog() {
	cap = 32;
	count = 0;
	events = new Events[cap];
}
EventLog::~EventLog() {
	delete[] events;
	events = nullptr;
}
void EventLog::add(int x, int y, int z, int w) {
	if (count == cap) {
		cap *= 2;
		Events* temp = new Events[cap];
		for (int i = 0; i < count; i++) temp[i] = events[i];
		delete[] events;
		events = temp;
	}
	events[count++] = Events(x, y, z, w);
}
int EventLog::size() const {
	return count;
}
const Events& EventLog::get(int i) const {
	return events[i];
}
void EventLog::clear() {
	count = 0;
}