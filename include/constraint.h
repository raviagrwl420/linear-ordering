#ifndef CONSTRAINT_H
#define CONSTRAINT_H

class Constraint {
	public:
		int i, j, k;
		float value;
		bool active = true;

		Constraint () {};
		Constraint (int i, int j, int k, float value): i(i), j(j), k(k), value(value) {};

		void deactivate () {
			active = false;
		};

		bool operator < (Constraint);

};

#endif