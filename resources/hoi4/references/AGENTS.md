# Hearts of Iron IV Event Modding

## Reference Wikis

- [Event Modding](https://hoi4.paradoxwikis.com/Event_modding)
- [Triggers](https://hoi4.paradoxwikis.com/Triggers)
- [Effect](https://hoi4.paradoxwikis.com/Effect)

## Local References

Structured reference files (condensed from wiki tables) are in `references/effects/` — one file per category.

- **Effect_Scopes.txt** — every/random/all country, state, leader, character, division loops
- **Dual_Scopes.txt** — ROOT, FROM, PREV, THIS, TAG, event_target, var, mio, sp
- **Any_scope_*.txt** — General, Border wars, Variables, Arrays
- **Country_scope_*.txt** — 28 files covering General, States, Mana, Politics, Diplomacy, War, etc.
- **State_scope_*.txt** — General, Buildings, Resistance, Raids
- **Character_scope.txt** — General, Unit leaders, Country leaders, Combat, Operatives
- **Division_scope.txt**, **MIO_scope.txt**, **Contract_scope.txt**, **Raid_scope.txt**, **Special_Project_scope.txt**
- **Flow_control.txt** — if/else, random_list, while loops
- **Meta_effects.txt**, **Scripted_effects.txt**

Structured trigger reference files are in `references/triggers/`:

- **Trigger_Scopes.txt** — all/any/every country, state, leader scope iterators
- **Dual_Scopes.txt** — ROOT, FROM, PREV, THIS, TAG, event_target, var, mio, sp
- **Flow_control_tools.txt** — AND, OR, NOT, if, count_triggers, custom_tooltip
- **Any_scope_*.txt** — General, Career profile, Variables, Debugging
- **Country_scope_*.txt** — 18 files covering General, Politics, War, Diplomacy, Military, etc.
- **State_scope_*.txt** — General, Resistance
- **Character_scope_*.txt** — General, Advisors, Country leaders, Unit leaders, Operatives, Scientists
- **Combat.txt**, **Division_scope.txt**, **MIO_scope.txt**, **Contract_scope.txt**, **Special_Projects.txt**
- **Meta_triggers.txt**, **Scripted_triggers.txt**

Use `@references/effects/<file>` or `@references/triggers/<file>` to attach or grep locally instead of web-fetching.

## Project File Structure

19 event files in `resources/hoi4/events/`, each typically covering a thematic domain:

| File | Namespace |
|------|-----------|
| `AcePilots.txt` | `ace_pilots` |
| `CapitulationEvents.txt` | `capitulation_events` |
| `Decisions_Events.txt` | `decision_events` |
| `ElectionEvents.txt` | `election_events` |
| `faction_alignment.txt` | `faction_alignment` |
| `generic_war.txt` | `generic_war` |
| `Generic.txt` | `generic` |
| `GOT_Generic_Events.txt` | `GOT_guarantee_events`, `GOT_newsevents` |
| `ideological_tension_v2.txt` | `tension_events` |
| `LaR_agent_events.txt` | various LaR namespaces |
| `LaR_espionage_operations.txt` | espionage namespaces |
| `NewsEvents.txt` | `news_events` |
| `NSB_NewsEvents.txt` | `nsb_news_events` |
| `NukeEvents.txt` | `nuke_events` |
| `Special_Project_Events.txt` | `special_project_events` |
| `stability_events.txt` | `stability`, `strikes_event`, `mutiny_event` |
| `TestEvents.txt` | `test`, `test_state`, `performance_test` |
| `WarJustification.txt` | `war_justification` |

## Namespace Rules

- Every event file must declare namespaces with `add_namespace = <name>` before any event using it
- Event IDs follow the pattern `<namespace>.<integer>` (e.g. `generic_war.1`)
- The integer ID must be numeric; non-integer IDs silently default to 0 and collide
- A file can declare multiple namespaces (e.g. `stability`, `strikes_event`, `mutiny_event`)
- Namespace can include dots for sub-topics: `add_namespace = my_event.subtopic`

## Event Types

### `country_event`
The most common type. ROOT = the country receiving the event. FROM = the country that fired it.

### `news_event`
A reskin of `country_event`. Requires `major = yes` to fire for all countries. Do NOT use `fire_only_once = yes` with news events — it prevents global delivery.

### `state_event`
ROOT = both the country and the state. Avoid when possible — use country_events with event targets to a specific state instead.

## Trigger Patterns (when events fire)

### `is_triggered_only = yes`
Event only fires via explicit effect (e.g. `country_event = { id = x days = 2 }`). No automatic MTTH checking. Used for:
- Focus/decision completion rewards
- Option-chained event sequences
- Delayed resolution events

### `mean_time_to_happen = { ... }`
Auto-fires with median delay. Supported units: `days`, `months` (30d), `years` (365d). Uses `modifier = { ... }` blocks with `factor` (multiply) and `add` (offset). Evaluated in order written.

### Flag-based gating
Used to prevent re-firing and to track event sequence state:
- `has_country_flag` / `set_country_flag` / `clr_country_flag`
- Flag-based dedup pattern: `NOT = { has_country_flag = my_flag }` in trigger, then `set_country_flag = my_flag` in option
- Timed flags: `set_country_flag = { flag = my_flag days = 90 }`
- Example: `WarJustification.txt` uses `war_justification_last_event_political`, `_neighbor`, `_mobilized`, `_borderthreat` — clear all others when setting one

### `fire_only_once = yes`
Global singleton — prevents ANY country from firing this event again (not per-country).

### `hidden = yes`
No pop-up; auto-picks first option. Used for:
- Delayed effect execution (hidden event + `country_event` with delay)
- Internal state transitions visible only via side-effects

## Event Body Structure

```
country_event = {
    id = namespace.N
    title = "localisation_key"
    desc = "localisation_key"
    picture = "GFX_sprite_name"

    trigger = { ... }           # conditions to fire
    mean_time_to_happen = { ... } # or is_triggered_only = yes

    immediate = { ... }         # runs before option is picked (tooltip visible)
    after = { ... }             # runs after option picked (patch 1.16.9+)

    option = {
        name = "loc_key"
        trigger = { ... }       # visibility condition for this option
        ai_chance = { ... }     # AI weight calculation
        ...
    }
}
```

## Conditional Localisation

For `title` and `desc` fields: supply multiple blocks with `trigger` conditions. The first matching block is used.

```
title = {
    text = my_event.1.t.a
    trigger = { tag = ENG }
}
title = {
    text = my_event.1.t.b  # fallback
}
```

## Scope Management

### Key Scopes
- **ROOT** = the country receiving the event (or primary scope)
- **FROM** = the country/scope that fired/spawned this event (can chain: FROM.FROM)
- **THIS** = current evaluation scope
- **PREV** = parent scope container
- **event_target:<key>** = saved scope via `save_event_target_as`

### Common Scope Patterns in This Project
- `FROM = { country_event = { id = other.N } }` — chain event to sender
- `event_target:ally = { ... }` — target saved scope from `immediate` block
- `ROOT` saves itself, FROM saves relevant states: `ROOT = { save_event_target_as = our_country } FROM = { save_event_target_as = their_state }`

## Effect Patterns (option/after/immediate)

### Event Chaining
- `country_event = { id = x days = N random_days = M }` — fire with 0-to-M random delay added to N
- For state events: `state_event = { id = x days = N trigger_for = TAG }`

### Opinion Modifiers
- `add_opinion_modifier = { target = ROOT modifier = refused_call_to_arms }`
- Opinion modifier keys: `refused_call_to_arms`, `came_to_our_aid` (from `generic_war.txt`)

### `ai_chance` Conventions
Proportional weight, automatically normalized. Use `base = N` as the default, then `modifier = { factor = X conditions }`. If all options have weight 0, first option is chosen.

Common modifier patterns:
- `has_war_support > 0.5` → factor 1.3-1.5
- `has_stability < 0.4` → factor 0 (block option) or factor 2 (encourage negotation)
- `surrender_progress > 0.2` → factor 1.5

### Variable Usage
- `set_variable = { name = value_source }` / `check_variable = { var > value }`
- Variables can store numeric values from game metrics (e.g. `deployed_army_manpower_k`)
- Can be used as effect values: `add_manpower = var:my_var`

### Tooltip Control
- `custom_effect_tooltip = "loc_key"` — replace tooltip for contained effects
- `custom_override_tooltip = { tooltip = "key" effects }` — full override (also works as trigger)
- `hidden_trigger = { ... }` — hide triggers from tooltip
- `effect_tooltip = { ... }` — show effects in tooltip without executing them

## Common Mistakes to Avoid

1. **Missing `add_namespace`** → "Malformed token" error, event won't work
2. **Non-integer event ID** → silently treated as `namespace.0`, collides with other non-integer IDs
3. **News event without `major = yes`** → fires for only one country
4. **News event with `fire_only_once = yes`** → only one country ever sees it
5. **MTTH event with no country check** → fires for wrong country (taglist order matters)
6. **`is_triggered_only` + `mean_time_to_happen`** → contradictory, logged error
7. **Tight date bounds in MTTH event** → likely never fires (checked every 20 days)
8. **Integer ID >= 100000** → ID collision across namespaces