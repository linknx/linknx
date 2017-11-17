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
		// As the current thread has not been spawned by pth, looks like
		// pthsem events do not work. We have to observe the action's finished
		// state instead.
		while (!isFinished())
		{
			pth_sleep(1);
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
		std::cout << "setUp" << std::endl;
		pth_init();
        rule_m = new TestableRule();
    }

    void tearDown()
    {
		// Make sure we exit all threads before we delete the rule.
		//pth_exit(0);
		std::cout << "tearDown" << std::endl;
        delete rule_m; rule_m = NULL;
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

		// Rule has not been evaluated yet.
        CPPUNIT_ASSERT_EQUAL(0, action->getCounter());

		// Evaluate rule to execute action.
		rule_m->evaluate();
		action->waitForCompletion();
        CPPUNIT_ASSERT_EQUAL(1, action->getCounter());

		// If rule is evaluated again, action is executed once again.
		rule_m->evaluate();
		action->waitForCompletion();
        CPPUNIT_ASSERT_EQUAL(2, action->getCounter());
    }

    void testOnTrueActionList()
    {
		CounterAction *action = new CounterAction();
		rule_m->addAction(action, Rule::OnTrue); 
		rule_m->getCondition()->setValue(true);

		// Rule has not been evaluated yet.
        CPPUNIT_ASSERT_EQUAL(0, action->getCounter());

		// Evaluate rule to execute action.
		rule_m->evaluate();
		action->waitForCompletion();
        CPPUNIT_ASSERT_EQUAL(1, action->getCounter());

		// If rule is evaluated again, action is NOT executed a second time.
		rule_m->evaluate();
		action->waitForCompletion();
        CPPUNIT_ASSERT_EQUAL(1, action->getCounter());
    }
	
    void testIfFalseActionList()
    {
		CounterAction *action = new CounterAction();
		rule_m->addAction(action, Rule::IfFalse); 
		rule_m->getCondition()->setValue(false);

		// Rule has not been evaluated yet.
        CPPUNIT_ASSERT_EQUAL(0, action->getCounter());

		// Evaluate rule to execute action.
		rule_m->evaluate();
		action->waitForCompletion();
        CPPUNIT_ASSERT_EQUAL(1, action->getCounter());

		// If rule is evaluated again, action is executed once again.
		rule_m->evaluate();
		action->waitForCompletion();
        CPPUNIT_ASSERT_EQUAL(2, action->getCounter());
    }
	
	// TODO Does not pass because rule is initialized to false (this is the
	// default). There is no value change, then!
    void testOnFalseActionList()
    {
		CounterAction *action = new CounterAction();
		rule_m->addAction(action, Rule::OnFalse); 
		rule_m->getCondition()->setValue(false);

		// Rule has not been evaluated yet.
        CPPUNIT_ASSERT_EQUAL(0, action->getCounter());

		// Evaluate rule to execute action.
		rule_m->evaluate();
		action->waitForCompletion();
        CPPUNIT_ASSERT_EQUAL(1, action->getCounter());

		// If rule is evaluated again, action is NOT executed a second time.
		rule_m->evaluate();
		action->waitForCompletion();
        CPPUNIT_ASSERT_EQUAL(1, action->getCounter());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( RuleTest );
