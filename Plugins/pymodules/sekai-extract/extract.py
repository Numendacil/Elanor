import json, sys, os

from UnityPy.classes import (
    Object,
    MonoBehaviour,
    TextAsset,
    Font,
    Shader,
    Mesh,
    Sprite,
    Texture2D,
    AudioClip,
)
from UnityPy.enums.ClassIDType import ClassIDType
from typing import Union, List

# Disable
def BlockPrint():
    sys.stdout = open(os.devnull, 'w')

# Restore
def EnablePrint():
    sys.stdout = sys.__stdout__

def exportTextAsset(obj: TextAsset, fp: str):
	with open(f"{fp}", "wb") as f:
		f.write(obj.script)


def exportFont(obj: Font, fp: str):
	if obj.m_FontData:
		with open(f"{fp}", "wb") as f:
			f.write(obj.m_FontData)


def exportMesh(obj: Mesh, fp: str):
	with open(f"{fp}", "wt", encoding="utf8", newline="") as f:
		f.write(obj.export())


def exporShader(obj: Shader, fp: str):
	with open(f"{fp}", "wt", encoding="utf8", newline="") as f:
		f.write(obj.export())


def exportMonoBehaviour(obj: Union[MonoBehaviour, Object], fp: str):
	export = None
	if obj.serialized_type and obj.serialized_type.nodes:
		export = json.dumps(obj.read_typetree(), indent=4, ensure_ascii=False).encode(
			"utf8", errors="surrogateescape"
		)
	else:
		export = obj.raw_data
	with open(f"{fp}", "wb") as f:
		f.write(export)


# def exportAudioClip(obj: AudioClip, fp: str):
# 	samples = obj.samples
# 	if len(samples) == 0:
# 		pass
# 	elif len(samples) == 1:
# 		with open(f"{fp}", "wb") as f:
# 			f.write(list(samples.values())[0])
# 	else:
# 		os.makedirs(fp, exist_ok=True)
# 		for name, clip_data in samples.items():
# 			with open(os.path.join(fp, f"{name}.wav"), "wb") as f:
# 				f.write(clip_data)


def exportSprite(obj: Sprite, fp: str):
	if obj.image.mode in ("RGBA", "P"):
		fp = os.path.splitext(fp)[0] + '.png'

	obj.image.save(f"{fp}")


def exportTexture2D(obj: Texture2D, fp: str):
	if obj.m_Width:
		# textures can be empty
		if obj.image.mode in ("RGBA", "P"):
			fp = os.path.splitext(fp)[0] + '.png'
		obj.image.save(f"{fp}")


EXPORT_TYPES = {
    ClassIDType.Sprite: exportSprite,
#     ClassIDType.AudioClip: exportAudioClip,
    ClassIDType.Font: exportFont,
    ClassIDType.Mesh: exportMesh,
    ClassIDType.MonoBehaviour: exportMonoBehaviour,
    ClassIDType.Shader: exporShader,
    ClassIDType.TextAsset: exportTextAsset,
    ClassIDType.Texture2D: exportTexture2D,
}
