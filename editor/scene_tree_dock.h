/*************************************************************************/
/*  scene_tree_dock.h                                                    */
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
#ifndef SCENE_TREE_DOCK_H
#define SCENE_TREE_DOCK_H

#include "connections_dialog.h"
#include "create_dialog.h"
#include "editor_data.h"
#include "editor_sub_scene.h"
#include "groups_editor.h"
#include "reparent_dialog.h"
#include "scene/animation/animation_player.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/control.h"
#include "scene/gui/label.h"
#include "scene/gui/popup_menu.h"
#include "scene/gui/tool_button.h"
#include "scene/gui/tree.h"
#include "scene_tree_editor.h"
#include "script_create_dialog.h"
class EditorNode;

class SceneTreeDock : public VBoxContainer {

	OBJ_TYPE(SceneTreeDock, VBoxContainer);

	enum Tool {

		TOOL_NEW,
		TOOL_INSTANCE,
		TOOL_REPLACE,
		TOOL_CONNECT,
		TOOL_GROUP,
		TOOL_ATTACH_SCRIPT,
		TOOL_CLEAR_SCRIPT,
		TOOL_MOVE_UP,
		TOOL_MOVE_DOWN,
		TOOL_DUPLICATE,
		TOOL_REPARENT,
		TOOL_NEW_SCENE_FROM,
		TOOL_MERGE_FROM_SCENE,
		TOOL_MULTI_EDIT,
		TOOL_ERASE,
		TOOL_COPY_NODE_PATH,
		TOOL_BUTTON_MAX
	};

	int current_option;
	CreateDialog *create_dialog;

	ToolButton *button_add;
	ToolButton *button_instance;
	ToolButton *button_create_script;
	ToolButton *button_clear_script;

	SceneTreeEditor *scene_tree;

	HBoxContainer *tool_hbc;
	void _tool_selected(int p_tool, bool p_confirm_override = false);

	EditorData *editor_data;
	EditorSelection *editor_selection;

	//GroupsEditor *groups_editor;
	//ConnectionsDialog *connect_dialog;
	ScriptCreateDialog *script_create_dialog;
	AcceptDialog *accept;
	ConfirmationDialog *delete_dialog;

	ReparentDialog *reparent_dialog;
	EditorFileDialog *file;
	EditorSubScene *import_subscene_dialog;
	EditorFileDialog *new_scene_from_dialog;

	LineEdit *filter;
	TextureFrame *filter_icon;

	PopupMenu *menu;

	bool first_enter;

	void _create();
	Node *scene_root;
	Node *edited_scene;
	EditorNode *editor;

	Node *_duplicate(Node *p_node, Map<Node *, Node *> &duplimap);
	void _node_reparent(NodePath p_path, bool p_keep_global_xform);
	void _do_reparent(Node *p_new_parent, int p_position_in_parent, Vector<Node *> p_nodes, bool p_keep_global_xform);

	void _set_owners(Node *p_owner, const Array &p_nodes);
	void _load_request(const String &p_path);
	void _script_open_request(const Ref<Script> &p_script);

	bool _cyclical_dependency_exists(const String &p_target_scene_path, Node *p_desired_node);

	void _node_selected();
	void _node_renamed();
	void _script_created(Ref<Script> p_script);

	void _delete_confirm();

	void _node_prerenamed(Node *p_node, const String &p_new_name);

	void _unhandled_key_input(InputEvent p_event);

	void _import_subscene();

	void _new_scene_from(String p_file);

	bool _validate_no_foreign();
	void _selection_changed();

	void _fill_path_renames(Vector<StringName> base_path, Vector<StringName> new_base_path, Node *p_node, List<Pair<NodePath, NodePath> > *p_renames);

	void _normalize_drop(Node *&to_node, int &to_pos, int p_type);

	void _nodes_dragged(Array p_nodes, NodePath p_to, int p_type);
	void _files_dropped(Vector<String> p_files, NodePath p_to, int p_type);
	void _script_dropped(String p_file, NodePath p_to);

	void _tree_rmb(const Vector2 &p_menu_pos);

	void _filter_changed(const String &p_filter);

	void _perform_instance_scenes(const Vector<String> &p_files, Node *parent, int p_pos);
	void _replace_with_branch_scene(const String &p_file, Node *base);

	void _file_selected(String p_file);

	String _new_node_name(String p_type);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	String get_filter();
	void set_filter(const String &p_filter);

	void import_subscene();
	void set_edited_scene(Node *p_scene);
	void instance(const String &p_path);
	void instance_scenes(const Vector<String> &p_files, Node *p_parent = NULL);
	void set_selected(Node *p_node, bool p_emit_selected = false);
	void fill_path_renames(Node *p_node, Node *p_new_parent, List<Pair<NodePath, NodePath> > *p_renames);
	void perform_node_renames(Node *p_base, List<Pair<NodePath, NodePath> > *p_renames, Map<Ref<Animation>, Set<int> > *r_rem_anims = NULL);
	SceneTreeEditor *get_tree_editor() { return scene_tree; }
	EditorData *get_editor_data() { return editor_data; }

	void open_script_dialog(Node *p_for_node);
	SceneTreeDock(EditorNode *p_editor, Node *p_scene_root, EditorSelection *p_editor_selection, EditorData &p_editor_data);
};

#endif // SCENE_TREE_DOCK_H
