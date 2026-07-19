{
	'variables': {
		'bin': '<!(node -e "import(\'@node-3d/addon-tools\').then((m) => m.printBin())")',
		'gl_include': '<!(node -p "require(\'@node-3d/deps-opengl\').include")',
		'gl_bin': '<!(node -p "require(\'@node-3d/deps-opengl\').bin")',
	},
	'targets': [{
		'target_name': 'glfw',
		'includes': ['common.gypi'],
		'sources': [
			'cpp/bindings.cpp',
		],
		'include_dirs': [
			'<(gl_include)',
			'<!@(node -e "import(\'@node-3d/addon-tools\').then((m) => m.printInclude())")',
			'include',
		],
		'library_dirs': ['<(gl_bin)'],
		'conditions': [
			['OS=="linux"', {
				'libraries': [
					"-Wl,-rpath,'$$ORIGIN'",
					"-Wl,-rpath,'$$ORIGIN/../node_modules/@node-3d/deps-opengl/<(bin)'",
					"-Wl,-rpath,'$$ORIGIN/../../@node-3d/deps-opengl/<(bin)'",
					'<(gl_bin)/libglfw.so.3',
					'<(gl_bin)/libGL.so',
					'<(gl_bin)/libXrandr.so',
				],
			}],
			['OS=="mac"', {
				'defines': ['GL_SILENCE_DEPRECATION'],
				'libraries': [
					'-Wl,-rpath,@loader_path',
					'-Wl,-rpath,@loader_path/../node_modules/@node-3d/deps-opengl/<(bin)',
					'-Wl,-rpath,@loader_path/../../@node-3d/deps-opengl/<(bin)',
					'<(gl_bin)/libglfw.3.dylib',
				],
			}],
			['OS=="win"', {
				'libraries': ['glfw3dll.lib', 'opengl32.lib'],
			}],
		],
	}],
}
