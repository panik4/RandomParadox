#!/usr/bin/env python3
"""Parse HOI4 Triggers wiki page HTML into category files."""
import re
import os

html_file = r"C:\Users\Pizza\.local\share\opencode\tool-output\tool_f5ca74d4a001bs05eTkcAaysUX"
out_dir = r"D:\Documents\Visual Studio 2017\Projects\RandomParadox\resources\hoi4\references\triggers"

with open(html_file, 'r', encoding='utf-8') as f:
    html = f.read()

# Extract headings with positions
headings = []
for m in re.finditer(r'<h([234])[^>]*><span class="mw-headline" id="([^"]+)"', html):
    headings.append({'level': int(m.group(1)), 'id': m.group(2), 'index': m.start()})

# Build parent h2 for each heading
headings_sorted = sorted(headings, key=lambda h: h['index'])
parent_stack = []
heading_parents = {}
for h in headings_sorted:
    while parent_stack and parent_stack[-1]['level'] >= h['level']:
        parent_stack.pop()
    parent_stack.append(h)
    h2 = next((p for p in parent_stack if p['level'] == 2), None)
    heading_parents[h['id']] = h2['id'] if h2 else ''

# Extract all wikitable elements
def extract_tables(html):
    tables = []
    pattern = re.compile(r'<table[^>]*class="[^"]*wikitable[^"]*"[^>]*>', re.IGNORECASE)
    for m in pattern.finditer(html):
        start = m.start()
        pos = m.end()
        depth = 1
        while depth > 0 and pos < len(html) - 8:
            if html[pos:pos+8].lower() == '</table>':
                depth -= 1
                if depth == 0: pos += 8; break
                pos += 8
            elif html[pos:pos+6].lower() == '<table':
                depth += 1; pos += 6
            else:
                pos += 1
        if depth == 0:
            tables.append({'start': start, 'end': pos, 'html': html[start:pos]})
    return tables

all_tables = extract_tables(html)
print(f"Total wikitable elements: {len(all_tables)}")

# Filter to top-level tables with data rows
top_tables = []
for i, t in enumerate(all_tables):
    is_nested = any(t['start'] > o['start'] and t['end'] < o['end']
                    for j, o in enumerate(all_tables) if i != j)
    if not is_nested and re.search(r'<tr[^>]*>\s*<td', t['html']):
        top_tables.append(t)
print(f"Top-level wikitables with data: {len(top_tables)}")

# Category mapping
cat_mapping = {
    'Trigger_scopes|Scopes': 'Trigger_Scopes',
    'Dual_scopes|Scopes': 'Dual_Scopes',
    'Flow_control_tools|Flow control tools': 'Flow_control_tools',
    'General|Any_scope': 'Any_scope_General',
    'Career_profile|Any_scope': 'Any_scope_Career_Profile',
    'Variables|Any_scope': 'Any_scope_Variables',
    'Debugging|Any_scope': 'Any_scope_Debugging',
    'General_2|Country_scope': 'Country_scope_General',
    'National_focuses|Country_scope': 'Country_scope_National_Focuses',
    'Politics|Country_scope': 'Country_scope_Politics',
    'Balance_of_power|Country_scope': 'Country_scope_Balance_of_Power',
    'Buildings|Country_scope': 'Country_scope_Buildings',
    'Technology|Country_scope': 'Country_scope_Technology',
    'Ideas|Country_scope': 'Country_scope_Ideas',
    'Diplomacy|Country_scope': 'Country_scope_Diplomacy',
    'Faction|Country_scope': 'Country_scope_Faction',
    'War|Country_scope': 'Country_scope_War',
    'State|Country_scope': 'Country_scope_State',
    'Military|Country_scope': 'Country_scope_Military',
    'Doctrine|Country_scope': 'Country_scope_Doctrine',
    'Equipment|Country_scope': 'Country_scope_Equipment',
    'Intelligence|Country_scope': 'Country_scope_Intelligence',
    'AI|Country_scope': 'Country_scope_AI',
    'Characters|Country_scope': 'Country_scope_Characters',
    'Peace_conferences|Country_scope': 'Country_scope_Peace_Conferences',
    'Faction_scope|Faction scope': 'Faction_scope',
    'General_3|State_scope': 'State_scope_General',
    'Resistance_and_Compliance|State_scope': 'State_scope_Resistance',
    'General_4|Character_scope': 'Character_scope_General',
    'Advisors|Character_scope': 'Character_scope_Advisors',
    'Country_leaders|Character_scope': 'Character_scope_Country_Leaders',
    'Unit_leaders|Character_scope': 'Character_scope_Unit_Leaders',
    'Operatives|Character_scope': 'Character_scope_Operatives',
    'Scientists|Character_scope': 'Character_scope_Scientists',
    'Combat': 'Combat',
    'Division_scope': 'Division_scope',
    'MIO_scope': 'MIO_scope',
    'Contract_scope': 'Contract_scope',
    'Special_projects': 'Special_Projects',
    'Flow_control_tools': 'Flow_control_tools',
    'Meta_triggers': 'Meta_triggers',
    'Scripted_triggers': 'Scripted_triggers',
}
# Subsections
cat_mapping[u'Useful_scripted_triggers|Scripted_triggers'] = 'Scripted_triggers'

def get_cat_name(sec_id):
    h2 = heading_parents.get(sec_id, '')
    key = f"{sec_id}|{h2}" if h2 and h2 != sec_id else sec_id
    result = cat_mapping.get(key)
    if result is None:
        result = cat_mapping.get(sec_id)
    return result

def clean_text(text):
    pre = re.search(r'<pre[^>]*>(.*?)</pre>', text, re.DOTALL)
    if pre:
        text = pre.group(1)
    else:
        text = re.sub(r'<br\s*/?>', ' ', text)
        text = re.sub(r'<[^>]+>', '', text)
    text = text.replace('&lt;','<').replace('&gt;','>').replace('&amp;','&')
    text = text.replace('&#8230;','...').replace('&#91;','[').replace('&#93;',']')
    text = text.replace('&#39;',"'").replace('&quot;','"').replace('&nbsp;',' ')
    text = re.sub(r'\s+', ' ', text).strip()
    return text

def get_caption(table_html):
    m = re.search(r'<caption[^>]*>(.*?)</caption>', table_html, re.DOTALL)
    if m:
        text = re.sub(r'<[^>]+>', '', m.group(1))
        text = text.replace('&nbsp;', ' ')
        return re.sub(r'\s+', ' ', text).strip()
    return ''

# Detect column layout from header row
def detect_layout(table_html):
    caption = get_caption(table_html)
    first_row = re.search(r'<tr[^>]*>(.*?)</tr>', table_html, re.DOTALL)
    if not first_row:
        return 'general'
    headers = [clean_text(c) for c in re.findall(r'<th[^>]*>(.*?)</th>', first_row.group(1), re.DOTALL)]
    hdr_txt = '|'.join(headers)
    
    if 'Target type' in hdr_txt:
        return 'scope'  # Name | Usage | Target type | Example | Description | Version Added
    return 'general'  # Name | Parameters | Examples | Description | Notes/Version

def parse_table(table_html, layout='general'):
    entries = []
    rows = re.findall(r'<tr[^>]*>(.*?)</tr>', table_html, re.DOTALL)
    for ri, row in enumerate(rows):
        if ri == 0:
            continue
        cells = re.findall(r'<td[^>]*>(.*?)</td>', row, re.DOTALL)
        cols = [clean_text(c) for c in cells]
        if len(cols) < 2 or not cols[0] or cols[0] == 'Name':
            continue

        entry = {'name': cols[0]}
        if layout == 'scope':
            if len(cols) >= 2: entry['Scope'] = cols[1]
            if len(cols) >= 3: entry['Target'] = cols[2]
            if len(cols) >= 4: entry['Example'] = cols[3]
            if len(cols) >= 5: entry['Desc'] = cols[4]
            if len(cols) >= 6: entry['Since'] = cols[-1]
        else:
            if len(cols) >= 2: entry['Params'] = cols[1]
            if len(cols) >= 3: entry['Example'] = cols[2]
            if len(cols) >= 4: entry['Desc'] = cols[3]
            if len(cols) >= 6: entry['Since'] = cols[-1]
        entries.append(entry)
    return entries

# Process all tables
cat_entries = {}
unmapped = set()

for tbl in top_tables:
    sec_id = ''
    for h in sorted(headings, key=lambda h: h['index'], reverse=True):
        if h['index'] < tbl['start']:
            sec_id = h['id']
            break

    cat_name = get_cat_name(sec_id)
    if not cat_name:
        unmapped.add(sec_id)
        continue

    layout = detect_layout(tbl['html'])
    entries = parse_table(tbl['html'], layout)

    if cat_name not in cat_entries:
        cat_entries[cat_name] = []
    cat_entries[cat_name].extend(entries)

print(f"\nMapped: {sum(len(v) for v in cat_entries.values())} entries across {len(cat_entries)} categories")
if unmapped:
    print(f"Unmapped sections: {', '.join(sorted(unmapped))}")

# Write files
os.makedirs(out_dir, exist_ok=True)
key_order = ['Scope', 'Target', 'Params', 'Example', 'Desc', 'Since']
for cat in sorted(cat_entries.keys()):
    entries = cat_entries[cat]
    lines = [f"# {cat}", "# Source: https://hoi4.paradoxwikis.com/Triggers", ""]
    for e in entries:
        lines.append(f"## {e['name']}")
        for k in key_order:
            if k in e:
                lines.append(f"- {k}: {e[k]}")
        lines.append("")
    filepath = os.path.join(out_dir, f"{cat}.txt")
    with open(filepath, 'w', encoding='utf-8') as f:
        f.write('\n'.join(lines))
    print(f"  {cat}.txt ({len(entries)} entries)")

print("\nDone!")