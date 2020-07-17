bl_info = {
	"name": "JSD Format Exporter",
	"description": "Exports JSD ((Jacajack's) JSON Scene Description) files",
	"author": "Jacek Wieczorek",
	"version": (1, 0),
	"blender": (2, 80, 0),
	"location": "File > Export > JSD",
	"warning": "",
	"category": "Import-Export"
}

import bpy, bmesh
import json
from mathutils import *
from math import *

C = bpy.context
D = bpy.data

# Redirect prints to all consoles in Blender
def print(data):
	for window in bpy.context.window_manager.windows:
		screen = window.screen
		for area in screen.areas:
			if area.type == 'CONSOLE':
				override = {'window': window, 'screen': screen, 'area': area}
				bpy.ops.console.scrollback_append(override, text=str(data), type="OUTPUT")

# Converts material (as in bpy.data.materials) to my format
def convert_material(material):
	mat_data = {
		'base_color': [1, 1, 1],
		'transmission': 0,
		'roughness': 0.5,
		'metallic': 0,
		'ior': 1.5,
		'emission': [0, 0, 0]
	};
	
	# Principled BSDF
	bsdf = material.node_tree.nodes.get("Principled BSDF")
	if bsdf:
		mat_data['base_color'] = list(bsdf.inputs['Base Color'].default_value)
		mat_data['emission'] = list(bsdf.inputs['Emission'].default_value)
		mat_data['roughness'] = bsdf.inputs['Roughness'].default_value
		mat_data['metallic'] = bsdf.inputs['Metallic'].default_value
		mat_data['transmission'] = bsdf.inputs['Transmission'].default_value
		mat_data['ior'] = bsdf.inputs['IOR'].default_value
		
	# Emission BSDF
	bsdf = material.node_tree.nodes.get("Emission")
	if bsdf:
		st = bsdf.inputs['Strength'].default_value
		col = bsdf.inputs['Color'].default_value
		mat_data['emission'] = [col[0] * st, col[1] * st, col[2] * st]
	
	# Glass BSDF
	bsdf = material.node_tree.nodes.get("Glass BSDF")
	if bsdf:
		mat_data['transmission'] = 1
		mat_data['base_color'] = list(bsdf.inputs['Color'].default_value)
		mat_data['ior'] = bsdf.inputs['IOR'].default_value
	
	return mat_data

# Converts object given as an argument to JSON representation
def convert_object(o):
	obj = {}
	
	# Object name
	obj['name'] = o.name
	print(o.name)
	
	# Materials
	obj['materials'] = []
	for mat_slot in o.material_slots:
		obj['materials'].append(convert_material(mat_slot.material))
	
	# Det dependency graph
	dg = bpy.context.evaluated_depsgraph_get()
	
	# Triangulate (create a new mesh form the object and usign the deps graph)
	bm = bmesh.new()
	bm.from_object(o, dg)
	bmesh.ops.triangulate(bm, faces=bm.faces[:], quad_method = 'BEAUTY', ngon_method = 'BEAUTY')
	bm.verts.ensure_lookup_table()
	bm.faces.ensure_lookup_table()
	
	mat = o.matrix_world
	
	# Vertex data
	obj['vertices'] = []
	for v in bm.verts:   
		vco = mat @ v.co
		vno = v.normal.to_4d()
		vno.w = 0
		vno = (mat @ vno).to_3d()
		vert = {}
		vert['p'] = [vco.x, vco.z, -vco.y] # X/Z swap
		vert['n'] = [vno.x, vno.z, -vno.y]
		obj['vertices'].append(vert)
		
	# Faces data
	obj['faces'] = []
	for f in bm.faces:
		face = {}
		face['vi'] = []
		for v in f.verts:
			face['vi'].append(v.index);
		face['mat_id'] = f.material_index
		face['sm'] = f.smooth
		N = f.normal.to_4d()
		N.w = 0
		N = (mat @ N).to_3d()
		face['n'] = [N.x, N.z, -N.y]
		obj['faces'].append(face)
	
	# Free the mesh
	bm.free()
	return obj
	
# Converts active scene to JSON format
def convert_scene(scene):
	data = {}
	data['objects'] = []
	
	# Iterate over meshes
	for o in scene.objects:
		if (o.type == 'MESH'):
			data['objects'].append(convert_object(o))

	return data;

# --------------- Blender Exporter Registration ---------------- 

class ObjectExport(bpy.types.Operator):
	bl_idname = "object.export_jsd"
	bl_label = "Export JSD file"
	bl_options = {'REGISTER', 'UNDO'}
	filename_ext = ".jsd"
	
	filename = bpy.props.StringProperty(subtype = 'FILE_PATH')
	
	def execute(self, context):
		scene_data = convert_scene(context.scene)
		f = open(self.filename, 'w')
		f.write(json.dumps(scene_data))
		f.close()
		return {'FINISHED'}
	
	def invoke(self, context, event):
		context.window_manager.fileselect_add(self)
		return {'RUNNING_MODAL'}
	
def menu_func_export(self, context):
	self.layout.operator(ObjectExport.bl_idname, text = "JSD File Export (.jsd)")

def register():
	bpy.utils.register_class(ObjectExport)
	bpy.types.TOPBAR_MT_file_export.append(menu_func_export)
	
def unregister():
	bpy.utils.unregister_class(ObjectExport)
	bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)
	
if __name__ == "__main__":
	register()