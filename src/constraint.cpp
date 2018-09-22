#include<constraint.h>

bool Constraint::operator < (Constraint other) {
	return this->value > other.value;
}