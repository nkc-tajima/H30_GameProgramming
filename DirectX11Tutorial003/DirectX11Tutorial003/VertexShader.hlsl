// ---------------------------------------------------------------------------------
// GT3A&GK1A �Q�[���v���O���~���O
// �O�p�`��`��F���_�V�F�[�_
// ---------------------------------------------------------------------------------
// CPU����󂯎�钸�_�o�b�t�@
struct VS_IN
{
	// D3D11_INPUT_ELEMENT_DESC�Œ�`�����Z�}���e�B�N�X���E�C���f�b�N�X�ԍ�
	float4 pos : POSITION0;
	float4 col : COLOR0;
};
// �s�N�Z���V�F�[�_�ɓn���l
struct VS_OUT
{
	// ���ɒ��_�V�F�[�_�ŏ���������Ă���̂�
	// �V�X�e����ň������W���Ƃ���SV_POSITION��n��
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
};
// �G���g���[�|�C���g
VS_OUT main(VS_IN input)
{
	VS_OUT output;

	output.pos = input.pos;
	output.col = input.col;
	return output;
}