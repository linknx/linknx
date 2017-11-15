#include <cppunit/extensions/HelperMacros.h>
#include "timermanager.h"
#include "services.h"
#include <iostream>

class ConstantCondition : public Condition
{
public:
	ConstantCondition(bool value) : value_m(value)
	{
	}

public:
	virtual void importXml(ticpp::Element* pConfig) {}
	virtual void exportXml(ticpp::Element* pConfig) {}
	virtual void statusXml(ticpp::Element* pStatus) {}
	virtual bool evaluate() {return value_m;}
	
	void setValue(bool value) {value_m = value;}

private:
	bool value_m;
};

class CounterAction : public Action
{
public:
	CounterAction() : counter_m(0) {}
	
    virtual void importXml(ticpp::Element* pConfig);
    virtual void Run (pth_sem_t * stop);

	int getCounter() const {return counter_m;}

private:
	int counter_m;
}

class TestableRule : public Rule
{
public:
    TestableRule() : Rule() {};

public:
	ConstantCondition* getCondition() const
	{
		return dynamic_cast<ConstantCondition*>(Rule::getCondition());
	}
};

class RuleTest : public CppUnit::TestFixture/*, public ChangeListener*/
{
    CPPUNIT_TEST_SUITE( RuleTest );
    CPPUNIT_TEST( testIfTrueActionlist );
    
    CPPUNIT_TEST_SUITE_END();

private:
    TestableRule *rule_m;

public:
    void setUp()
    {
        rule_m = new TestableRule();
		rule_m->setCondition(new ConstantCondition(true));
		
    }

    void tearDown()
    {
        delete rule_m;
    }

    /*void onChange(Object* obj)
    {
        isOnChangeCalled_m = true;
    }*/

    void testIfTrueActionList()
    {
		CounterAction *action = new CounterAction();
		rule_m->addAction(action, Rule::IfTrue); 
		rule_m->getCondition()->setValue(true);

        CPPUNIT_ASSERT_EQUAL(0, action->getCounter());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( RuleTest );
