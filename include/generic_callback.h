#ifndef GENERIC_CALLBACK_H
#define GENERIC_CALLBACK_H

#include<io.h>

#include<ilcplex/ilocplex.h>

class GenericCallback: public IloCplex::Callback::Function {
	private:
		GenericCallback () {};

		IloArray<IloBoolVarArray> x;
		int** matrix;

	public:
		GenericCallback (const IloArray<IloBoolVarArray> &x, int** &matrix): x(x), matrix(matrix) {};

		void invoke (const IloCplex::Callback::Context &context);

		virtual ~GenericCallback();
};

#endif