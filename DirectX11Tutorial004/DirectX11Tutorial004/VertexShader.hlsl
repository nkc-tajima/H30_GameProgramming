// �󂯎�钸�_�f�[�^�̒l
struct VS_IN
{
	float4 pos : POSITION0;// D3D11_INPUT_ELEMENT_DESC�Œ�`�����Z�}���e�B�N�X���E�C���f�b�N�X�ԍ��̗v�f���󂯎��
	float4 col : COLOR0;
};
// �s�N�Z���V�F�[�_�[�ɑ���l
struct VS_OUT
{
	// ���ɒ��_�V�F�[�_�ŏ���������Ă���̂ŁA�V�X�e����ň������W���Ƃ���SV_POSITION�ɂȂ�
	// SV��System Value�̗��H
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
};

VS_OUT main(VS_IN input)
{
	VS_OUT output;

	output.pos = input.pos;
	output.col = input.col;
	return output;
}