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
	CounterAction(int increment) : counter_m(0), increment_m(increment) {}
	
    virtual void importXml(ticpp::Element* pConfig) {}
	virtual void Run (pth_sem_t * stop)
	{
		counter_m += increment_m;
	}

	int getCounter() const {return counter_m;}
	void waitForCompletion()
	{
		timespec spec;
		spec.tv_nsec = 10000000; // In nanoseconds => 10ms.

		while (!isFinished())
		{
			pth_nanosleep(&spec, NULL);
		}
	}

private:
	int counter_m;
	int increment_m;
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
    CPPUNIT_TEST( testIfTrueAndOnTrueActionLists );
    
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
		testOneActionList(true, ActionList::IfTrue, 2);
    }

    void testOnTrueActionList()
    {
		testOneActionList(true, ActionList::OnTrue, 1);
    }
	
    void testIfFalseActionList()
    {
		testOneActionList(false, ActionList::IfFalse, 2);
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

		testOneActionList(false, ActionList::OnFalse, 1);
    }

	/** This test is used to reproduce issue 33 */
    void testIfTrueAndOnTrueActionLists()
    {
		CounterAction *action1 = new CounterAction(1);
		rule_m->addAction(action1, ActionList::OnTrue); 
		CounterAction *action2 = new CounterAction(10);
		rule_m->addAction(action2, ActionList::IfTrue); 

		rule_m->getCondition()->setValue(true);

		// Rule has not been evaluated yet.
        CPPUNIT_ASSERT_EQUAL(0, action1->getCounter());
        CPPUNIT_ASSERT_EQUAL(0, action2->getCounter());

		// Evaluate rule to execute both actions.
		rule_m->evaluate();
		action1->waitForCompletion();
		action2->waitForCompletion();
        CPPUNIT_ASSERT_EQUAL(1, action1->getCounter());
        CPPUNIT_ASSERT_EQUAL(10, action2->getCounter());

		// If rule is evaluated again, only action2 is executed again.
		rule_m->evaluate();
		action1->waitForCompletion();
		action2->waitForCompletion();
        CPPUNIT_ASSERT_EQUAL(1, action1->getCounter());
        CPPUNIT_ASSERT_EQUAL(20, action2->getCounter());
    }
	

private:
    void testOneActionList(bool condition, ActionList::TriggerType type, int expectedFinalCount)
    {
		CounterAction *action = new CounterAction(1);
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
