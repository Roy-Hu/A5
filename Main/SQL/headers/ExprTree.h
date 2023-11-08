
#ifndef SQL_EXPRESSIONS
#define SQL_EXPRESSIONS

#include "MyDB_AttType.h"
#include <string>
#include <vector>
#include "MyDB_Catalog.h"

// create a smart pointer for database tables
using namespace std;
class ExprTree;
typedef shared_ptr <ExprTree> ExprTreePtr;

// this class encapsules a parsed SQL expression (such as "this.that > 34.5 AND 4 = 5")

// class ExprTree is a pure virtual class... the various classes that implement it are below

enum TYPE{
	TYPE_NUMBER,
	TYPE_BOOL,
	TYPE_STRING,
	TYPE_IDENTIFIER,
	TYPE_UNKNOW
};


class ExprTree {

public:
	virtual string toString () = 0;
	virtual ~ExprTree () {}

	virtual bool checkQuery (MyDB_CatalogPtr myCatalog, vector <pair <string, string>> tablesToProcess) {
		return true;
	};

	virtual TYPE getType (){
		return TYPE_UNKNOW;
	}
};

class BoolLiteral : public ExprTree {

private:
	bool myVal;
public:
	
	BoolLiteral (bool fromMe) {
		myVal = fromMe;
	}

	string toString () {
		if (myVal) {
			return "bool[true]";
		} else {
			return "bool[false]";
		}
	}	

	TYPE getType (){
		return TYPE_BOOL;
	}

};

class DoubleLiteral : public ExprTree {

private:
	double myVal;
public:

	DoubleLiteral (double fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "double[" + to_string (myVal) + "]";
	}	

	TYPE getType (){
		return TYPE_NUMBER;
	}

	~DoubleLiteral () {}
};

// this implement class ExprTree
class IntLiteral : public ExprTree {

private:
	int myVal;
public:

	IntLiteral (int fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "int[" + to_string (myVal) + "]";
	}


	TYPE getType (){
		return TYPE_NUMBER;
	}

	~IntLiteral () {}
};

class StringLiteral : public ExprTree {

private:
	string myVal;
public:

	StringLiteral (char *fromMe) {
		fromMe[strlen (fromMe) - 1] = 0;
		myVal = string (fromMe + 1);
	}

	string toString () {
		return "string[" + myVal + "]";
	}

	TYPE getType (){
		return TYPE_STRING;
	}

	~StringLiteral () {}
};

class Identifier : public ExprTree {

private:
	string tableName;
	string attName;
	TYPE type;
public:

	Identifier (char *tableNameIn, char *attNameIn) {
		tableName = string (tableNameIn);
		attName = string (attNameIn);
		type = TYPE_IDENTIFIER;
	}

	string toString () {
		return "[" + tableName + "_" + attName + "]";
	}	

	TYPE getType (){
		return type;
	}

	bool checkQuery (MyDB_CatalogPtr myCatalog, vector <pair <string, string>> tablesToProcess) {
		string attType;
		string tableFullName;
		bool found = false;

		for (auto t : tablesToProcess) {
			if (tableName.compare(t.second) == 0) {
				tableFullName = t.first;
				found = true;
				break;
			}	
		}

		if (!found) {
			printf("Err: referenced table %s do not exist in the catalog\n", tableName.c_str());
			return false;
		}		

		if (!myCatalog->getString(tableFullName + "." + attName + ".type", attType)) {
			printf("Err: attribute %s do not exist in trhe table %s\n", attName.c_str(), tableName.c_str());
			return false;
		}

		if (attType.compare("int") == 0 || attType.compare("double") == 0) {
			type = TYPE_NUMBER;
		} else if (attType.compare("string") == 0) {
			type = TYPE_STRING;
		} else if (attType.compare("bool") == 0) {
			type = TYPE_BOOL;
		}

		return true;
	}

	~Identifier () {}
};

class MinusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	MinusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "- (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	TYPE getType () {
		return TYPE_NUMBER;
	}

	bool checkQuery (MyDB_CatalogPtr myCatalog, vector <pair <string, string>> tablesToProcess) {
		if (!lhs->checkQuery(myCatalog, tablesToProcess) || !rhs->checkQuery(myCatalog, tablesToProcess)) {
			return false;
		}

		if (!(lhs->getType() == TYPE_NUMBER && rhs->getType() == TYPE_NUMBER)) {
			printf("Err: lhs %s and rhs %s should both be NUMBER to use minus operator -\n",
					 lhs->toString().c_str(), rhs->toString().c_str());
			return false;
		}

		return true;
	}

	~MinusOp () {}
};

class PlusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	TYPE type;
	
public:

	PlusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
		type = TYPE_UNKNOW;
	}

	string toString () {
		return "+ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	TYPE getType () {
		return type;
	}

	bool checkQuery (MyDB_CatalogPtr myCatalog, vector <pair <string, string>> tablesToProcess) {
		if (!lhs->checkQuery(myCatalog, tablesToProcess) || !rhs->checkQuery(myCatalog, tablesToProcess)) {
			return false;
		}

		if (lhs->getType() == TYPE_NUMBER && rhs->getType() == TYPE_NUMBER) {
			type = TYPE_NUMBER;
		} else if (lhs->getType() == TYPE_STRING && rhs->getType() == TYPE_STRING) {
			type = TYPE_STRING;
		} else {
			printf("Err: lhs %s and rhs %s should both be NUMBER or STRING to use plus operator +\n",
					 lhs->toString().c_str(), rhs->toString().c_str());
			return false;
		}

		return true;
	}

	~PlusOp () {}
};

class TimesOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	TimesOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "* (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	TYPE getType () {
		return TYPE_NUMBER;
	}

	bool checkQuery (MyDB_CatalogPtr myCatalog, vector <pair <string, string>> tablesToProcess) {
		if (!lhs->checkQuery(myCatalog, tablesToProcess) || !rhs->checkQuery(myCatalog, tablesToProcess)) {
			return false;
		}
	
		if (!(lhs->getType() == TYPE_NUMBER && rhs->getType() == TYPE_NUMBER)) {
			printf("Err: lhs %s and rhs %s should both be NUMBER to use time operator *\n",
					 lhs->toString().c_str(), rhs->toString().c_str());
			return false;
		}

		return true;
	}

	~TimesOp () {}
};

class DivideOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	DivideOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "/ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	TYPE getType () {
		return TYPE_NUMBER;
	}
	
	bool checkQuery (MyDB_CatalogPtr myCatalog, vector <pair <string, string>> tablesToProcess) {
		if (!lhs->checkQuery(myCatalog, tablesToProcess) || !rhs->checkQuery(myCatalog, tablesToProcess)) {
			return false;
		}

		if (!(lhs->getType() == TYPE_NUMBER && rhs->getType() == TYPE_NUMBER)) {
			printf("Err: lhs %s and rhs %s should both be NUMBER to use divide operator /\n",
					 lhs->toString().c_str(), rhs->toString().c_str());
			return false;
		}

		return true;
	}

	~DivideOp () {}
};

class GtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	GtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "> (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	TYPE getType () {
		return TYPE_BOOL;
	}

	bool checkQuery (MyDB_CatalogPtr myCatalog, vector <pair <string, string>> tablesToProcess) {
		if (!lhs->checkQuery(myCatalog, tablesToProcess) || !rhs->checkQuery(myCatalog, tablesToProcess)) {
			return false;
		}

		if (lhs->getType() != rhs->getType()) {
			printf("Err: lhs %s and rhs %s should both be same type to use greater than operator >\n",
					 lhs->toString().c_str(), rhs->toString().c_str());
			return false;
		}

		return true;
	}

	~GtOp () {}
};

class LtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	LtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "< (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	TYPE getType () {
		return TYPE_BOOL;
	}

	bool checkQuery (MyDB_CatalogPtr myCatalog, vector <pair <string, string>> tablesToProcess) {
		if (!lhs->checkQuery(myCatalog, tablesToProcess) || !rhs->checkQuery(myCatalog, tablesToProcess)) {
			return false;
		}

		if (lhs->getType() != rhs->getType()) {
			printf("Err: lhs %s and rhs %s should both be same type to use less than operator <\n",
					 lhs->toString().c_str(), rhs->toString().c_str());
			return false;
		}

		return true;
	}

	~LtOp () {}
};

class NeqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	NeqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "!= (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	TYPE getType () {
		return TYPE_BOOL;
	}

	bool checkQuery (MyDB_CatalogPtr myCatalog, vector <pair <string, string>> tablesToProcess) {
		if (!lhs->checkQuery(myCatalog, tablesToProcess) || !rhs->checkQuery(myCatalog, tablesToProcess)) {
			return false;
		}

		if (lhs->getType() != rhs->getType()) {
			printf("Err: lhs %s and rhs %s should both be same type to use not equal operator !=\n",
					 lhs->toString().c_str(), rhs->toString().c_str());
			return false;
		}

		return true;
	}

	~NeqOp () {}
};

class OrOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	OrOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "|| (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	TYPE getType () {
		return TYPE_BOOL;
	}

	bool checkQuery (MyDB_CatalogPtr myCatalog, vector <pair <string, string>> tablesToProcess) {
		if (!lhs->checkQuery(myCatalog, tablesToProcess) || !rhs->checkQuery(myCatalog, tablesToProcess)) {
			return false;
		}

		if (lhs->getType() != rhs->getType()) {
			printf("Err: lhs %s and rhs %s should both be same type to use or operator ||\n",
					 lhs->toString().c_str(), rhs->toString().c_str());
			return false;
		}

		return true;
	}

	~OrOp () {}
};

class EqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	EqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "== (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	TYPE getType () {
		return TYPE_BOOL;
	}

	bool checkQuery (MyDB_CatalogPtr myCatalog, vector <pair <string, string>> tablesToProcess) {
		if (!lhs->checkQuery(myCatalog, tablesToProcess) || !rhs->checkQuery(myCatalog, tablesToProcess)) {
			return false;
		}

		if (lhs->getType() != rhs->getType()) {
			printf("Err: lhs %s and rhs %s should both be same type to use equal operator ==\n",
					 lhs->toString().c_str(), rhs->toString().c_str());
			return false;
		}

		return true;
	}

	~EqOp () {}
};

class NotOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	NotOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "!(" + child->toString () + ")";
	}	

	TYPE getType () {
		return TYPE_BOOL;
	}

	bool checkQuery (MyDB_CatalogPtr myCatalog, vector <pair <string, string>> tablesToProcess) {
		if (!child->checkQuery(myCatalog, tablesToProcess)) {
			return false;
		}

		if (child->getType() != TYPE_BOOL) {
			printf("Err: Expression %s should be BOOL to use not operator !\n", 
					child->toString().c_str());
			return false;
		}

		return true;
	}

	~NotOp () {}
};

class SumOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	SumOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "sum(" + child->toString () + ")";
	}	

	bool checkQuery (MyDB_CatalogPtr myCatalog, vector <pair <string, string>> tablesToProcess) {
		if (!child->checkQuery(myCatalog, tablesToProcess)) {
			return false;
		}

		if (child->getType() != TYPE_NUMBER) {
			printf("Err: Child %s should be NUMBER to use sum operator\n",
					 child->toString().c_str());
			return false;
		}

		return true;
	}


	~SumOp () {}
};

class AvgOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	AvgOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "avg(" + child->toString () + ")";
	}	

	bool checkQuery (MyDB_CatalogPtr myCatalog, vector <pair <string, string>> tablesToProcess) {
		if (!child->checkQuery(myCatalog, tablesToProcess)) {
			return false;
		}

		if (child->getType() != TYPE_NUMBER) {
			printf("Err: Child %s should be NUMBER to use avg operator\n", 
					child->toString().c_str());
			return false;
		}

		return true;
	}

	~AvgOp () {}
};

#endif
