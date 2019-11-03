/*************************************************************************/
/*  dependency_editor.cpp                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#include "dependency_editor.h"
#include "editor_node.h"
#include "io/resource_loader.h"
#include "os/file_access.h"
#include "scene/gui/margin_container.h"

void DependencyEditor::_notification(int p_what) {
}

void DependencyEditor::_searched(const String &p_path) {

	Map<String, String> dep_rename;
	dep_rename[replacing] = p_path;

	ResourceLoader::rename_dependencies(editing, dep_rename);

	_update_list();
	_update_file();
}

void DependencyEditor::_load_pressed(Object *p_item, int p_cell, int p_button) {

	TreeItem *ti = p_item->cast_to<TreeItem>();
	String fname = ti->get_text(0);
	replacing = ti->get_text(1);

	search->set_title(TTR("Search Replacement For:") + " " + replacing.get_file());

	search->clear_filters();
	List<String> ext;
	ResourceLoader::get_recognized_extensions_for_type(ti->get_metadata(0), &ext);
	for (List<String>::Element *E = ext.front(); E; E = E->next()) {
		search->add_filter("*" + E->get());
	}
	search->popup_centered_ratio();
}

void DependencyEditor::_fix_and_find(EditorFileSystemDirectory *efsd, Map<String, Map<String, String> > &candidates) {

	for (int i = 0; i < efsd->get_subdir_count(); i++) {
		_fix_and_find(efsd->get_subdir(i), candidates);
	}

	for (int i = 0; i < efsd->get_file_count(); i++) {

		String file = efsd->get_file(i);
		if (!candidates.has(file))
			continue;

		String path = efsd->get_file_path(i);

		for (Map<String, String>::Element *E = candidates[file].front(); E; E = E->next()) {

			if (E->get() == String()) {
				E->get() = path;
				continue;
			}

			//must match the best, using subdirs
			String existing = E->get().replace_first("res://", "");
			String current = path.replace_first("res://", "");
			String lost = E->key().replace_first("res://", "");

			Vector<String> existingv = existing.split("/");
			existingv.invert();
			Vector<String> currentv = current.split("/");
			currentv.invert();
			Vector<String> lostv = lost.split("/");
			lostv.invert();

			int existing_score = 0;
			int current_score = 0;

			for (int j = 0; j < lostv.size(); j++) {

				if (j < existingv.size() && lostv[j] == existingv[j]) {
					existing_score++;
				}
				if (j < currentv.size() && lostv[j] == currentv[j]) {
					current_score++;
				}
			}

			if (current_score > existing_score) {

				//if it was the same, could track distance to new path but..

				E->get() = path; //replace by more accurate
			}
		}
	}
}

void DependencyEditor::_fix_all() {

	if (!EditorFileSystem::get_singleton()->get_filesystem())
		return;

	Map<String, Map<String, String> > candidates;

	for (List<String>::Element *E = missing.front(); E; E = E->next()) {

		String base = E->get().get_file();
		if (!candidates.has(base)) {
			candidates[base] = Map<String, String>();
		}

		candidates[base][E->get()] = "";
	}

	_fix_and_find(EditorFileSystem::get_singleton()->get_filesystem(), candidates);

	Map<String, String> remaps;

	for (Map<String, Map<String, String> >::Element *E = candidates.front(); E; E = E->next()) {

		for (Map<String, String>::Element *F = E->get().front(); F; F = F->next()) {

			if (F->get() != String()) {
				remaps[F->key()] = F->get();
			}
		}
	}

	if (remaps.size()) {

		ResourceLoader::rename_dependencies(editing, remaps);

		_update_list();
		_update_file();
	}
}

void DependencyEditor::_update_file() {

	EditorFileSystem::get_singleton()->update_file(editing);
}

void DependencyEditor::_update_list() {

	List<String> deps;
	ResourceLoader::get_dependencies(editing, &deps, true);

	tree->clear();
	missing.clear();

	TreeItem *root = tree->create_item();

	Ref<Texture> folder = get_icon("folder", "FileDialog");

	bool broken = false;

	for (List<String>::Element *E = deps.front(); E; E = E->next()) {

		TreeItem *item = tree->create_item(root);

		String n = E->get();
		String path;
		String type;

		if (n.find("::") != -1) {
			path = n.get_slice("::", 0);
			type = n.get_slice("::", 1);
		} else {
			path = n;
			type = "Resource";
		}
		String name = path.get_file();

		Ref<Texture> icon;
		if (has_icon(type, "EditorIcons")) {
			icon = get_icon(type, "EditorIcons");
		} else {
			icon = get_icon("Object", "EditorIcons");
		}
		item->set_text(0, name);
		item->set_icon(0, icon);
		item->set_metadata(0, type);
		item->set_text(1, path);

		if (!FileAccess::exists(path)) {
			item->set_custom_color(1, Color(1, 0.4, 0.3));
			missing.push_back(path);
			broken = true;
		}

		item->add_button(1, folder, 0);
	}

	fixdeps->set_disabled(!broken);
}

void DependencyEditor::edit(const String &p_path) {

	editing = p_path;
	set_title(TTR("Dependencies For:") + " " + p_path.get_file());

	_update_list();
	popup_centered_ratio();

	if (EditorNode::get_singleton()->is_scene_open(p_path)) {
		EditorNode::get_singleton()->show_warning(vformat(TTR("Scene '%s' is currently being edited.\nChanges will not take effect unless reloaded."), p_path.get_file()));
	} else if (ResourceCache::has(p_path)) {
		EditorNode::get_singleton()->show_warning(vformat(TTR("Resource '%s' is in use.\nChanges will take effect when reloaded."), p_path.get_file()));
	}
}

void DependencyEditor::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("_searched"), &DependencyEditor::_searched);
	ObjectTypeDB::bind_method(_MD("_load_pressed"), &DependencyEditor::_load_pressed);
	ObjectTypeDB::bind_method(_MD("_fix_all"), &DependencyEditor::_fix_all);
}

DependencyEditor::DependencyEditor() {

	VBoxContainer *vb = memnew(VBoxContainer);
	vb->set_name(TTR("Dependencies"));
	add_child(vb);
	set_child_rect(vb);

	tree = memnew(Tree);
	tree->set_columns(2);
	tree->set_column_titles_visible(true);
	tree->set_column_title(0, TTR("Resource"));
	tree->set_column_title(1, TTR("Path"));
	tree->set_hide_root(true);
	tree->connect("button_pressed", this, "_load_pressed");

	HBoxContainer *hbc = memnew(HBoxContainer);
	Label *label = memnew(Label(TTR("Dependencies:")));
	hbc->add_child(label);
	hbc->add_spacer();
	fixdeps = memnew(Button(TTR("Fix Broken")));
	hbc->add_child(fixdeps);
	fixdeps->connect("pressed", this, "_fix_all");

	vb->add_child(hbc);

	MarginContainer *mc = memnew(MarginContainer);
	mc->set_v_size_flags(SIZE_EXPAND_FILL);

	mc->add_child(tree);
	vb->add_child(mc);

	set_title(TTR("Dependency Editor"));
	search = memnew(EditorFileDialog);
	search->connect("file_selected", this, "_searched");
	search->set_mode(EditorFileDialog::MODE_OPEN_FILE);
	search->set_title(TTR("Search Replacement Resource:"));
	add_child(search);
}

/////////////////////////////////////
void DependencyEditorOwners::_list_rmb_select(int p_item, const Vector2 &p_pos) {

	file_options->clear();
	file_options->set_size(Size2(1, 1));
	if (p_item >= 0) {
		file_options->add_item(TTR("Open"), FILE_OPEN);
	}

	file_options->set_pos(owners->get_global_pos() + p_pos);
	file_options->popup();
}

void DependencyEditorOwners::_select_file(int p_idx) {

	String fpath = owners->get_item_text(p_idx);

	if (ResourceLoader::get_resource_type(fpath) == "PackedScene") {
		editor->open_request(fpath);
		hide();
		emit_signal("confirmed");
	}
}

void DependencyEditorOwners::_file_option(int p_option) {

	switch (p_option) {
		case FILE_OPEN: {
			int idx = owners->get_current();
			if (idx < 0 || idx >= owners->get_item_count())
				break;
			_select_file(idx);
		} break;
	}
}

void DependencyEditorOwners::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("_list_rmb_select"), &DependencyEditorOwners::_list_rmb_select);
	ObjectTypeDB::bind_method(_MD("_file_option"), &DependencyEditorOwners::_file_option);
	ObjectTypeDB::bind_method(_MD("_select_file"), &DependencyEditorOwners::_select_file);
}

void DependencyEditorOwners::_fill_owners(EditorFileSystemDirectory *efsd) {

	if (!efsd)
		return;

	for (int i = 0; i < efsd->get_subdir_count(); i++) {
		_fill_owners(efsd->get_subdir(i));
	}

	for (int i = 0; i < efsd->get_file_count(); i++) {

		Vector<String> deps = efsd->get_file_deps(i);
		//print_line(":::"+efsd->get_file_path(i));
		bool found = false;
		for (int j = 0; j < deps.size(); j++) {
			//print_line("\t"+deps[j]+" vs "+editing);
			if (deps[j] == editing) {
				//print_line("found");
				found = true;
				break;
			}
		}
		if (!found)
			continue;

		Ref<Texture> icon;
		String type = efsd->get_file_type(i);
		if (!has_icon(type, "EditorIcons")) {
			icon = get_icon("Object", "EditorIcons");
		} else {
			icon = get_icon(type, "EditorIcons");
		}

		owners->add_item(efsd->get_file_path(i), icon);
	}
}

void DependencyEditorOwners::show(const String &p_path) {

	editing = p_path;
	owners->clear();
	_fill_owners(EditorFileSystem::get_singleton()->get_filesystem());
	popup_centered_ratio();

	set_title(TTR("Owners Of:") + " " + p_path.get_file());
}

DependencyEditorOwners::DependencyEditorOwners(EditorNode *p_editor) {

	editor = p_editor;

	file_options = memnew(PopupMenu);
	add_child(file_options);
	file_options->connect("item_pressed", this, "_file_option");

	owners = memnew(ItemList);
	owners->set_select_mode(ItemList::SELECT_SINGLE);
	owners->connect("item_rmb_selected", this, "_list_rmb_select");
	owners->connect("item_activated", this, "_select_file");
	owners->set_allow_rmb_select(true);

	add_child(owners);
	set_child_rect(owners);
}

///////////////////////

void DependencyRemoveDialog::_fill_owners(EditorFileSystemDirectory *efsd) {

	if (!efsd)
		return;

	for (int i = 0; i < efsd->get_subdir_count(); i++) {
		_fill_owners(efsd->get_subdir(i));
	}

	for (int i = 0; i < efsd->get_file_count(); i++) {

		Vector<String> deps = efsd->get_file_deps(i);
		//print_line(":::"+efsd->get_file_path(i));
		Set<String> met;
		for (int j = 0; j < deps.size(); j++) {
			if (files.has(deps[j])) {
				met.insert(deps[j]);
			}
		}
		if (!met.size())
			continue;

		exist = true;

		Ref<Texture> icon;
		String type = efsd->get_file_type(i);
		if (!has_icon(type, "EditorIcons")) {
			icon = get_icon("Object", "EditorIcons");
		} else {
			icon = get_icon(type, "EditorIcons");
		}

		for (Set<String>::Element *E = met.front(); E; E = E->next()) {

			String which = E->get();
			if (!files[which]) {
				TreeItem *ti = owners->create_item(owners->get_root());
				ti->set_text(0, which.get_file());
				files[which] = ti;
			}
			TreeItem *ti = owners->create_item(files[which]);
			ti->set_text(0, efsd->get_file_path(i));
			ti->set_icon(0, icon);
		}
	}
}

void DependencyRemoveDialog::show(const Vector<String> &to_erase) {

	exist = false;
	owners->clear();
	files.clear();
	owners->create_item(); // root
	for (int i = 0; i < to_erase.size(); i++) {
		files[to_erase[i]] = NULL;
	}

	_fill_owners(EditorFileSystem::get_singleton()->get_filesystem());

	if (exist) {
		owners->show();
		text->set_text(TTR("The files being removed are required by other resources in order for them to work.\nRemove them anyway? (no undo)"));
		popup_centered_minsize(Size2(500, 220));
	} else {
		owners->hide();
		text->set_text(TTR("Remove selected files from the project? (no undo)"));
		popup_centered_minsize(Size2(400, 100));
	}
}

void DependencyRemoveDialog::ok_pressed() {

	DirAccess *da = DirAccess::create(DirAccess::ACCESS_RESOURCES);
	for (Map<String, TreeItem *>::Element *E = files.front(); E; E = E->next()) {
		if (da->dir_exists(E->key())) {
			String path = OS::get_singleton()->get_resource_dir() + E->key().replace_first("res://", "/");
			OS::get_singleton()->move_path_to_trash(path);
			EditorFileSystem::get_singleton()->scan();
		} else {
			if (ResourceCache::has(E->key())) {
				Resource *res = ResourceCache::get(E->key());
				res->set_path(""); //clear reference to path
			}
			da->remove(E->key());
			EditorFileSystem::get_singleton()->update_file(E->key());
		}
	}
	memdelete(da);
}

DependencyRemoveDialog::DependencyRemoveDialog() {

	VBoxContainer *vb = memnew(VBoxContainer);
	add_child(vb);
	set_child_rect(vb);

	text = memnew(Label);
	vb->add_child(text);

	owners = memnew(Tree);
	owners->set_hide_root(true);
	vb->add_child(owners);
	owners->set_v_size_flags(SIZE_EXPAND_FILL);
	get_ok()->set_text(TTR("Remove"));
}

//////////////

void DependencyErrorDialog::show(const String &p_for_file, const Vector<String> &report) {

	for_file = p_for_file;
	set_title(TTR("Error loading:") + " " + p_for_file.get_file());
	files->clear();

	TreeItem *root = files->create_item(NULL);
	for (int i = 0; i < report.size(); i++) {

		String dep;
		String type = "Object";
		dep = report[i].get_slice("::", 0);
		if (report[i].get_slice_count("::") > 0)
			type = report[i].get_slice("::", 1);

		Ref<Texture> icon;
		if (!has_icon(type, "EditorIcons")) {
			icon = get_icon("Object", "EditorIcons");
		} else {
			icon = get_icon(type, "EditorIcons");
		}

		TreeItem *ti = files->create_item(root);
		ti->set_text(0, dep);
		ti->set_icon(0, icon);
	}

	popup_centered_minsize(Size2(500, 220));
}

void DependencyErrorDialog::ok_pressed() {

	EditorNode::get_singleton()->load_scene(for_file, true);
}

void DependencyErrorDialog::custom_action(const String &) {

	EditorNode::get_singleton()->fix_dependencies(for_file);
}

DependencyErrorDialog::DependencyErrorDialog() {

	VBoxContainer *vb = memnew(VBoxContainer);
	add_child(vb);
	set_child_rect(vb);

	files = memnew(Tree);
	files->set_hide_root(true);
	vb->add_margin_child(TTR("Scene failed to load due to missing dependencies:"), files, true);
	files->set_v_size_flags(SIZE_EXPAND_FILL);
	get_ok()->set_text(TTR("Open Anyway"));

	text = memnew(Label);
	vb->add_child(text);
	text->set_text(TTR("Which action should be taken?"));

	fdep = add_button(TTR("Fix Dependencies"), true, "fixdeps");

	set_title(TTR("Errors loading!"));
}

//////////////////////////////////////////////////////////////////////

void OrphanResourcesDialog::ok_pressed() {

	paths.clear();

	_find_to_delete(files->get_root(), paths);
	if (paths.empty())
		return;

	delete_confirm->set_text(vformat(TTR("Permanently delete %d item(s)? (No undo!)"), paths.size()));
	delete_confirm->popup_centered_minsize();
}

bool OrphanResourcesDialog::_fill_owners(EditorFileSystemDirectory *efsd, HashMap<String, int> &refs, TreeItem *p_parent) {

	if (!efsd)
		return false;

	bool has_childs = false;

	for (int i = 0; i < efsd->get_subdir_count(); i++) {

		TreeItem *dir_item = NULL;
		if (p_parent) {
			dir_item = files->create_item(p_parent);
			dir_item->set_text(0, efsd->get_subdir(i)->get_name());
			dir_item->set_icon(0, get_icon("folder", "FileDialog"));
		}
		bool children = _fill_owners(efsd->get_subdir(i), refs, dir_item);

		if (p_parent) {
			if (!children) {
				memdelete(dir_item);
			} else {
				has_childs = true;
			}
		}
	}

	for (int i = 0; i < efsd->get_file_count(); i++) {

		if (!p_parent) {
			Vector<String> deps = efsd->get_file_deps(i);
			//print_line(":::"+efsd->get_file_path(i));
			for (int j = 0; j < deps.size(); j++) {

				if (!refs.has(deps[j])) {
					refs[deps[j]] = 1;
				}
			}
		} else {

			String path = efsd->get_file_path(i);
			if (!refs.has(path)) {
				TreeItem *ti = files->create_item(p_parent);
				ti->set_cell_mode(0, TreeItem::CELL_MODE_CHECK);
				ti->set_text(0, efsd->get_file(i));
				ti->set_editable(0, true);

				String type = efsd->get_file_type(i);

				Ref<Texture> icon;
				if (has_icon(type, "EditorIcons")) {
					icon = get_icon(type, "EditorIcons");
				} else {
					icon = get_icon("Object", "EditorIcons");
				}
				ti->set_icon(0, icon);
				int ds = efsd->get_file_deps(i).size();
				ti->set_text(1, itos(ds));
				if (ds) {
					ti->add_button(1, get_icon("Visible", "EditorIcons"));
				}
				ti->set_metadata(0, path);
				has_childs = true;
			}
		}
	}

	return has_childs;
}

void OrphanResourcesDialog::refresh() {
	HashMap<String, int> refs;
	_fill_owners(EditorFileSystem::get_singleton()->get_filesystem(), refs, NULL);
	files->clear();
	TreeItem *root = files->create_item();
	_fill_owners(EditorFileSystem::get_singleton()->get_filesystem(), refs, root);
}

void OrphanResourcesDialog::show() {

	refresh();
	popup_centered_ratio();
}

void OrphanResourcesDialog::_find_to_delete(TreeItem *p_item, List<String> &paths) {

	while (p_item) {

		if (p_item->get_cell_mode(0) == TreeItem::CELL_MODE_CHECK && p_item->is_checked(0)) {
			paths.push_back(p_item->get_metadata(0));
		}

		if (p_item->get_children()) {
			_find_to_delete(p_item->get_children(), paths);
		}

		p_item = p_item->get_next();
	}
}

void OrphanResourcesDialog::_delete_confirm() {

	DirAccess *da = DirAccess::create(DirAccess::ACCESS_RESOURCES);
	for (List<String>::Element *E = paths.front(); E; E = E->next()) {

		da->remove(E->get());
		EditorFileSystem::get_singleton()->update_file(E->get());
	}
	memdelete(da);
	refresh();
}

void OrphanResourcesDialog::_button_pressed(Object *p_item, int p_column, int p_id) {

	TreeItem *ti = p_item->cast_to<TreeItem>();

	String path = ti->get_metadata(0);
	dep_edit->edit(path);
}

void OrphanResourcesDialog::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("_delete_confirm"), &OrphanResourcesDialog::_delete_confirm);
	ObjectTypeDB::bind_method(_MD("_button_pressed"), &OrphanResourcesDialog::_button_pressed);
}

OrphanResourcesDialog::OrphanResourcesDialog() {

	VBoxContainer *vbc = memnew(VBoxContainer);
	add_child(vbc);
	set_child_rect(vbc);
	files = memnew(Tree);
	files->set_columns(2);
	files->set_column_titles_visible(true);
	files->set_column_min_width(1, 100);
	files->set_column_expand(0, true);
	files->set_column_expand(1, false);
	files->set_column_title(0, TTR("Resource"));
	files->set_column_title(1, TTR("Owns"));
	files->set_hide_root(true);
	vbc->add_margin_child(TTR("Resources Without Explicit Ownership:"), files, true);
	set_title(TTR("Orphan Resource Explorer"));
	delete_confirm = memnew(ConfirmationDialog);
	delete_confirm->set_text(TTR("Delete selected files?"));
	get_ok()->set_text(TTR("Delete"));
	add_child(delete_confirm);
	dep_edit = memnew(DependencyEditor);
	add_child(dep_edit);
	files->connect("button_pressed", this, "_button_pressed");
	delete_confirm->connect("confirmed", this, "_delete_confirm");
	set_hide_on_ok(false);
}