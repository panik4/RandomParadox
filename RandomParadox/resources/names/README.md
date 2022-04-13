When modifying name generation, you have different options:
-Add new state names in state_types.txt. It is important to format them correctly. Separate them via ; and also ALWAYS contain template where the country name is supposed to be.
 Example: Revolutionary template; 
 However, you cannot add new ideologies/lines like democratic or monarchy. So no new lines.
- Editing token groups in token_groups.txt.  
  You can add token groups by adding a new line, e.g. :
	mytokenGroup;n;tt;r;s;reallyLongEntry;
	First, the name of the group. All entries afterwards are characters that appear randomly if you use this group in name_rules.txt. Each entry can be as long as you want.
  You can of course also edit all other token groups, remove and add characters, or remove a group altogether. However, if you remove a group, you need to remove it from name_rules.txt
  If you ever see an error mentioning a missing namegroup or token group, make sure to check for errors in the text files.
  There is no need to for capitalization of letters.
 
- Editing name rules:
  You can modify or add name rules. Name rules are randomly chosen for countries/regions etc. You can add as many name rules as you wish. Name rules work like this:
  For every entry separated by ; in the name rule, a random entry from a token group is selected. E.g.:
  E.g., the name rule: vowels;groupMiddle;vowels;groupEnd; gives:
   - a random element from vowels, e.g. o.
   - a random element from groupMiddle, e.g. lt.
   - a random element from vowels again, e.g. e.
   - a random element from groupEnd, e.g. stan.
  The full name now is Altestan.