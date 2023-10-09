from .merge_line import MergeLine as _MergeLine
from .merge_para import MergePara as _MergePara
from .merge_para_code import MergeParaCode as _MergeParaCode
from .merge_line_v_lr import MergeLineVlr as _MergeLineVlr
from .merge_line_v_rl import MergeLineVrl as _MergeLineVrl

MergeLine = _MergeLine().run
MergePara = _MergePara().run
MergeParaCode = _MergeParaCode().run
MergeLineVlr = _MergeLineVlr().run
MergeLineVrl = _MergeLineVrl().run
