//
//  ControlAssignment.h
//  Koumachika
//
//  Created by Toni on 4/16/19.
//
//

#ifndef ControlAssignment_h
#define ControlAssignment_h

template<typename E>
inline void handleControlAssignment(
	unordered_map<E,ControlActionState>& _map,
	const unordered_map<string, E>& nameMap,
	const string& line,
	const vector<string>& tokens,
	E _end,
	string typeName
){
	E keyButton = getOrDefaultUpperCase(nameMap, tokens.at(1), _end);
	if (keyButton == _end) {
		log2("control_mapping.txt: Unknown %s: %s", typeName.c_str(), tokens.at(1));
		return;
	}

	if (tokens.size() == 2) {
		log1("control_mapping.txt: \"%s\" ControlAction(s) missing", line.c_str());
		return;
	}

	ControlActionState result;
	bool valid = true;

	for (size_t i = 2; i < tokens.size(); ++i) {
		ControlAction action = getOrDefaultLowerCase(ControlRegister::actionNameMap, tokens.at(i), ControlAction::end);
		if (action == ControlAction::end) {
			valid = false;
			log1("control_mapping.txt: Unknown ControlAction %s", tokens.at(i).c_str());
		}
		else {
			result |= make_enum_bitfield(action);
		}
	}

	if (valid && result.any()) {
		_map.insert_or_assign(keyButton, result);
	}
	else {
		log1("control_mapping.txt: Invalid control assignment \"%s\" ignored.", line.c_str());
	}
}

#endif /* ControlAssignment_h */
