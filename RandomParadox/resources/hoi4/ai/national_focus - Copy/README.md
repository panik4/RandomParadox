National focus creation is customizable.
The generation works with chains:
  -  a chain is a set of events that are supposed to follow each other, but don't always have to.
  - you can define a chain for major powers, regional powers, weak powers or everyone
  - a chain has one to many steps like this: 
	-#chain ID;step;predecessor;rank;ideology;type;target;condition;skippable;exclusive;date;
	- Example:
	- 0;0;;major;fascism;attack;rank{weak}+ideology{not}+location{neighbour};yes;no;01-01-1936;
	- this chain step means that a major power, which is fascist, tries to find a target to attack that
		- is weak, not fascist, any a neighbour
		- this step can be skipped, so that the next step in the chain can also be run without this first step
		- it is also not exclusive with another step
	- the following steps follow:
		-0;1;0;major;fascism;attack;rank{regional}+ideology{same}+location{neighbour};no;yes;01-01-1936;
		-0;2;0;major;fascism;ally;rank{regional}+ideology{same}+location{neighbour};no;yes;01-01-1936;
		- now you can see here, that they have different step IDs, but the same predecessor, and are exclusive.
			- this means that only one of the foci will be selected into the chain. Either ally a fascist regional power or attack it
		
	
	
Supported requirements for sources:
	- ideology:fascism, communism, neutrality, democratic
	- rank: major, regional, weak
	
Supported focus types:
	- attack
	- todo ally
	- todo guarantee
	- todo create faction
	

Supported requirements for targets so far:
	- ideology:fascism, communism, neutrality, democratic, any
	- rank: major, regional, weak
	- location: neighbour, near, distant, any
	
	
Adding a new focus type:
	- think of a name, such as attack
	- create attackFocus.txt
	- put in normal paradox focus code
	- replace keywords by following supported keywords:
		- templateID
		- templateDestTag
		- templateSourceTag
		- templateXPosition instead of hardcoded x position
		- templateYPosition instead of hardcoded y position