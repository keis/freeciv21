Agents
******

An agent is a piece of code which is responsible for a certain area. An agent will be given a specification by
the user of the agent and a set of objects which the agent can control (the production queue of a city, a
city, a unit, a set of units or the whole empire). The user can be a human player or another part of the code
including another agent. There is no extra interaction between the user and the agent needed after the agent
got its task description.

Examples of agents:

* an agent which is responsible for moving a certain unit from A to B
* an agent which is responsible for maximize the food production of a city
* an agent which is responsible for the production queue of a city
* an agent which is responsible for defending a city
* an agent which is responsible for a city
* an agent which is responsible for all cities

An agent may use other agents to accomplish its goal. Such decencies form a hierarchy of agents. The position
in this hierarchy is denoted by a level. A higher level means more complexity. So an agent of level n can only
make use of agents of level (n-1) or lower. Level 0 defines actions which are carried out at the server and
are atomic actions (actions which can't be simulated at the client).

By such a definition an agent doesn't have to be implemented in C and also doesn't have to make use of
:file:`client/agents/agents.[cpp,h]`.

The core of an agent consist of two parts: a part which makes decisions and a part which carries out the
decision. The results of the first part should be made available. An agent lacking the execution part is
called advisor.

An agent should provide a GUI besides the core.

Implementation
==============

The received task description and any decision been made can be saved in attributes. An agent should not
assume anything. This includes especially: :strong:no magic numbers:strong:. Everything should be settable by
the user.

Use :file:`client/agents/agents.[cpp,h]` to get informed about certain events. Don't hesitate to add more
callbacks. Use :file:`client/agents/agents`:code:`wait_for_requests()` instead of
:file:`client/civclient`:code:`wait_till_request_got_processed()`.
