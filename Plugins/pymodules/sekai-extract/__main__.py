import os, pathlib
import UnityPy
UnityPy.config.FALLBACK_UNITY_VERSION = "2020.3.32f1"

from .extract import *

def GetInput() -> str:
	s = input()
	if s == "exit":
		print("exit")
		exit(0)
	return s

def UnpackObj(key: str, prefix: str, file: str):
	prefix = pathlib.Path(prefix)

	env = UnityPy.load(file)
	for path, obj in env.container.items():
		index = path.rfind(key)
		if index == -1:
			output_path = pathlib.Path(key)
			output_path = output_path / pathlib.PurePath(path).name
		else:
			output_path = pathlib.Path(path[index:])

		output_path = prefix / output_path
		os.makedirs(output_path.parent, exist_ok=True)

		export_func = EXPORT_TYPES.get(obj.type)
		if not export_func:
			with open(str(output_path) + '.raw', 'wb') as ofile:
				ofile.write(obj.get_raw_data())
		else:
			obj = obj.read()
			export_func(obj, output_path)

while True:
	try:
		s = ""
		while s != "start":
			s = GetInput()
		key = GetInput()
		prefix = GetInput()
		file = GetInput()
		end = GetInput()
		if end != "finish":
			raise Exception(message="Unexpected input: " + end)

		BlockPrint()

		UnpackObj(key, prefix, file)

		EnablePrint()
		print("ok")
	
	except Exception as e:
		EnablePrint()
		if hasattr(e, 'message'):
			print("Exception occured", e.message)
		else:
			print("Exception occured", e)

