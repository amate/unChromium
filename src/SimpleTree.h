/**
 *	@file	SimpleTree.h
 *	@brief	�ȈՃc���[�\����\�����邽�߂̃e���v���[�g
 *	@note
 *	�m�[�h�̏W���̂Ƃ��Ă̖؍\�����A�m�[�h�N���X�Ƃ��̊Ǘ��N���X��
 *	�\������B�m�[�h���Ɏ��f�[�^�̓e���v���[�g�ɂ���Ď��R�Ɍ���ł���̂�
 *	�l�X�Ȗ؍\���ɉ��p�o����B
 *	�������A�@�\�̓V���v���Ȃ̂Ŏ�Ԃ͂�����B
 */

#pragma once



/**
	CTreeNode<TNode>
	�m�[�h�Ƃ��ĕK�v�ȋ@�\�����b�v����N���X�B
	�e�m�[�h�ƑO��̃m�[�h�A����ɍŏ��̎q�ƍŌ�̎q��c������������B
	�����ăm�[�h���̏����e���v���[�g�����ɂ���Ď������邱�Ƃ��o����B

	TNode�Ƃ��ė^����^�̏����́A������Z�q�ɂ���Ė��Ȃ��f�[�^���R�s�[
	�����邩�ǂ����ł���BTNode�^�̒��Ƀ|�C���^�����݂���ꍇ��
	������Z�q���I�[�o�[���[�h���ē������������w���|�C���^������݂��Ȃ��悤
	���ӂ���K�v������B�R�s�[�R���X�g���N�^�͎g�p���Ȃ��͂������A�S�z�Ȃ�
	��`���Ă��悢�B
 */
template<class TNode>
class CTreeNode {
private:
public:
	TNode				Self;
	CTreeNode<TNode>*	pPrev,
					*	pNext,
					*	pParent,
					*	pFirstChild,
					*	pLastChild;

	CTreeNode() : Self()
	{
		pPrev = pNext = pParent = pFirstChild = pLastChild = NULL;
	}
};



/**
	CSimpleTree<TNode>
	�m�[�h���Ǘ����č\���𑀍삷�邽�߂̃N���X

	TNode�^�̏������m�[�h��؍\���Ƃ��ĊǗ��E���삷�邱�Ƃ��ł���B
	�����ł�CTreeNode<TNode>�̃|�C���^NODEPTR����đ��삷��B
 */
template<class TNode>
class CSimpleTree {
	//�^��`
	typedef CTreeNode<TNode>  NODE;
public:
	typedef NODE *			  NODEPTR;

private:
	//�����o�ϐ�
	NODE	m_Root; 					///< ���[�g�E�e�������Ȃ��B��̃m�[�h
	DWORD	m_dwNewCount;				///< ���������[�N�Ď��p�J�E���^

public:

	//�R���X�g���N�^
	CSimpleTree() : m_Root() , m_dwNewCount(0) { }

	//�f�X�g���N�^
	~CSimpleTree() { Clear(); }

	//���\�b�h

public:
	NODEPTR Insert(NODEPTR pParent, TNode &NewItem)
	{
		if (!pParent)
			pParent = &m_Root;

		NODEPTR 	pNewNode = AllocNode();
		pNewNode->Self		 = NewItem; 	//���g�̃R�s�[
		pNewNode->pParent	 = pParent;

		if (!pParent->pFirstChild) {
			//�ŏ��̎q�m�[�h�Ƃ��ēo�^
			pParent->pFirstChild = pNewNode;
			pParent->pLastChild  = pNewNode;
		} else {
			//�Ō�̎q�m�[�h�̌��ɂ�������
			NODEPTR  pLastChild = pParent->pLastChild;
			pLastChild->pNext	= pNewNode;
			pNewNode->pPrev 	= pLastChild;
			pParent->pLastChild = pNewNode;
		}

		return pNewNode;
	}


	BOOL Clear()
	{
		if (Delete(&m_Root) && m_dwNewCount == 0)
			return TRUE;

		ATLASSERT(FALSE);				//���������[�N���Ă���
		return FALSE;
	}


private:
	BOOL Delete(NODEPTR pNode)
	{
		ATLASSERT(pNode);

		//�܂��w��m�[�h�̎q��S�ď���
		if (pNode->pFirstChild) {
			DeleteChildren(pNode);
		}

		//�֘A�m�[�h�̂Ȃ����ύX����
		if (pNode->pParent) {
			if (pNode->pPrev) {
				//�O������B���Ȃ킿��낪����ΑO��2���Ȃ������ł悢
				if (pNode->pNext) {
					pNode->pPrev->pNext = pNode->pNext;
					pNode->pNext->pPrev = pNode->pPrev;
				} else {
					pNode->pPrev->pNext = NULL;
					pNode->pLastChild	= pNode->pPrev;
				}
			} else {
				//���ꂪ��Ԑ擪�̎q�m�[�h�B���Ȃ킿�e�Ƃ̊֌W���ύX����K�v������
				if (pNode->pNext) {
					pNode->pNext->pPrev 		= NULL;
					pNode->pParent->pFirstChild = pNode->pNext;
				} else {
					pNode->pParent->pFirstChild = NULL;
					pNode->pParent->pLastChild	= NULL;
				}
			}
		} else {
			//���[�g�ł���̂Ŕ�����
			return TRUE;
		}

		//����Ƃ̂Ȃ��肪�����Ȃ����Ƃ���ō폜����
		FreeNode(pNode);
		return TRUE;
	}


private:
	BOOL DeleteChildren(NODEPTR pNode)
	{
		ATLASSERT(pNode);

		NODEPTR pChild = pNode->pFirstChild;

		if (!pChild)
			return TRUE;

		while (pChild->pNext) {
			pChild = pChild->pNext;
			Delete(pChild->pPrev);
		}

		Delete(pChild);

		pNode->pFirstChild = NULL;
		pNode->pLastChild  = NULL;
		return TRUE;
	}


public:
	TNode *GetData(NODEPTR pNode)
	{
		return (TNode *) pNode; //���̂܂��
	}


private:
	NODEPTR GetRoot()
	{
		return &m_Root;
	}


public:
	//����֐� ���ڃ|�C���^���������ق�����������
	NODEPTR GetFirstChild(NODEPTR pNode)
	{
		if (!pNode)
			pNode = &m_Root;

		return pNode->pFirstChild;
	}


private:
	NODEPTR GetLastChild(NODEPTR pNode)
	{
		if (!pNode)
			pNode = &m_Root;

		return pNode->pLastChild :
	}


	NODEPTR GetPrev(NODEPTR pNode)
	{
		ATLASSERT(pNode);
		return pNode->pPrev;
	}


public:
	NODEPTR GetNext(NODEPTR pNode)
	{
		ATLASSERT(pNode);
		return pNode->pNext;
	}


private:
	NODEPTR GetParent(NODEPTR pNode)
	{
		ATLASSERT(pNode);
		return pNode->pParent;
	}


private:
	NODEPTR AllocNode()
	{
		++m_dwNewCount;
		return new NODE();
	}


	void FreeNode(NODEPTR pNode)
	{
		if (pNode) {
			--m_dwNewCount;
			delete pNode;
		}
	}
};
