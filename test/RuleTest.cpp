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
	
    virtual void importXml(ticpp::Element* pConfig) {}
	virtual void Run (pth_sem_t * stop)
	{
		++counter_m;
	}

	int getCounter() const {return counter_m;}
	void waitForCompletion()
	{
		timespec spec;
		spec.tv_nsec = 10000; // In nanoseconds.

		// As the current thread has not been spawned by pth, looks like
		// pthsem events do not work. We have to observe the action's finished
		// state instead.
		while (!isFinished())
		{
			pth_nanosleep(&spec, NULL);
		}
	}

private:
	int counter_m;
};

class TestableRule : public Rule
{
public:
    TestableRule() : Rule()
   	{
		setCondition(new ConstantCondition(true));
	}

public:
	ConstantCondition* getCondition() const
	{
		return dynamic_cast<ConstantCondition*>(Rule::getCondition());
	}
};

class RuleTest : public CppUnit::TestFixture/*, public ChangeListener*/
{
    CPPUNIT_TEST_SUITE( RuleTest );
    CPPUNIT_TEST( testIfTrueActionList );
    CPPUNIT_TEST( testOnTrueActionList );
    CPPUNIT_TEST( testIfFalseActionList );
    CPPUNIT_TEST( testOnFalseActionList );
    
    CPPUNIT_TEST_SUITE_END();

public:
	RuleTest() : rule_m(NULL) {}

private:
    TestableRule *rule_m;

public:
    void setUp()
    {
		pth_init();
        rule_m = new TestableRule();
    }

    void tearDown()
    {
        delete rule_m; rule_m = NULL;
    }

    /*void onChange(Object* obj)
    {
        isOnChangeCalled_m = true;
    }*/

    void testIfTrueActionList()
    {
		testActionList(true, Rule::IfTrue, 2);
    }

    void testOnTrueActionList()
    {
		testActionList(true, Rule::OnTrue, 1);
    }
	
    void testIfFalseActionList()
    {
		testActionList(false, Rule::IfFalse, 2);
    }
	
	// TODO Does not pass because rule is initialized to false (this is the
	// default). There is no value change, then!
    void testOnFalseActionList()
    {
		// Initialize rule to true, as default value is false.
		// Otherwise, the first evaluation below will not trigger
		// the action.
		rule_m->getCondition()->setValue(true);
		rule_m->evaluate();

		testActionList(false, Rule::OnFalse, 1);
    }

private:
    void testActionList(bool condition, Rule::ActionListTriggerType type, int expectedFinalCount)
    {
		CounterAction *action = new CounterAction();
		rule_m->addAction(action, type); 
		rule_m->getCondition()->setValue(condition);

		// Rule has not been evaluated yet.
        CPPUNIT_ASSERT_EQUAL(0, action->getCounter());

		// Evaluate rule to execute action.
		rule_m->evaluate();
		action->waitForCompletion();
        CPPUNIT_ASSERT_EQUAL(1, action->getCounter());

		// If rule is evaluated again, action is NOT executed a second time.
		rule_m->evaluate();
		action->waitForCompletion();
        CPPUNIT_ASSERT_EQUAL(expectedFinalCount, action->getCounter());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( RuleTest );
