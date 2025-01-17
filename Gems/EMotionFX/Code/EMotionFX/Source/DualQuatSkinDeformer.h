/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/std/containers/vector.h>
#include <AzCore/Outcome/Outcome.h>
#include "EMotionFXConfig.h"
#include <MCore/Source/DualQuaternion.h>
#include "Mesh.h"
#include "MeshDeformer.h"

namespace EMotionFX
{
    // forward declarations
    class Actor;
    class Node;

    /**
     * The soft skinning mesh deformer 'base' class.
     * The calculations will be done on the CPU, however there will also be
     * specialized versions of this class, so which are inherited from this class.
     * These other classes will be a special soft skinning deformer for the Intel Pentium 4 and one for the AMD Athlon.
     * However, this class will work on all CPUs, but will be slower as the natively optimized ones.
     */
    class EMFX_API DualQuatSkinDeformer
        : public MeshDeformer
    {
        AZ_CLASS_ALLOCATOR_DECL

    public:
        // the type id of the deformer, returned by GetType()
        enum
        {
            TYPE_ID    = 0x00000003
        };

        // the subtype id, returned by GetSubType()
        enum
        {
            SUBTYPE_ID = 0x00000002
        };

        /**
         * Creation method.
         * @param mesh A pointer to the mesh to deform.
         * @result The created object.
         */
        static DualQuatSkinDeformer* Create(Mesh* mesh);

        /**
         * Update the mesh deformer.
         * @param actorInstance The actor instance to use for the update. So the actor where the mesh belongs to during this update.
         * @param node The node to use for the update, so the node where the mesh belongs to during this update.
         * @param timeDelta The time (in seconds) passed since the last call.
         */
        void Update(ActorInstance* actorInstance, Node* node, float timeDelta) override;

        /**
         * Reinitialize the mesh deformer.
         * Updates the the array of bone information used for precalculation.
         * @param actor The actor to to initialize the deformer for.
         * @param node The node where the mesh belongs to during this initialization.
         * @param lodLevel The LOD level of the mesh the mesh deformer works on.
         */
        void Reinitialize(Actor* actor, Node* node, uint32 lodLevel) override;

        /**
         * Creates an exact clone (copy) of this deformer, and returns a pointer to it.
         * @param mesh The mesh to apply the deformer on.
         * @result A pointer to the newly created clone of this deformer.
         */
        MeshDeformer* Clone(Mesh* mesh) override;

        /**
         * Returns the unique type ID of the deformer.
         * @result The type ID of the deformer.
         */
        uint32 GetType() const override;

        /**
         * Returns the unique subtype ID of this deformer.
         * A subtype identifies the specialization type of the given deformer.
         * A cplus plus version of a deformer could have a sub type of 0, while the SSE assembly optimized
         * version would have a sub type of 1 for example.
         * @result The sub type identification number.
         */
        uint32 GetSubType() const override;

        /**
         * Get the number of bones used by this deformer.
         * This is the number of different bones that the skinning information of the mesh where this deformer works on uses.
         * @result The number of bones.
         */
        MCORE_INLINE uint32 GetNumLocalBones() const                        { return static_cast<uint32>(m_bones.size()); }

        /**
         * Get the node number of a given local bone.
         * @param index The local bone number, which must be in range of [0..GetNumLocalBones()-1].
         * @result The node number, which is in range of [0..Actor::GetNumNodes()-1], depending on the actor where this deformer works on.
         */
        MCORE_INLINE uint32 GetLocalBone(uint32 index) const                { return m_bones[index].mNodeNr; }

        /**
         * Pre-allocate space for a given number of local bones.
         * This does not alter the value returned by GetNumLocalBones().
         * @param numBones The number of bones to pre-allocate space for.
         */
        MCORE_INLINE void ReserveLocalBones(uint32 numBones)                { m_bones.reserve(numBones); }

    protected:
        /**
         * Structure used for pre-calculating the skinning matrices.
         */
        struct EMFX_API BoneInfo
        {
            uint32                  mNodeNr;        /**< The node number. */
            MCore::DualQuaternion   mDualQuat;      /**< The dual quat of the pre-calculated matrix that contains the "globalMatrix * inverse(bindPoseMatrix)". */

            MCORE_INLINE BoneInfo()
                : mNodeNr(MCORE_INVALIDINDEX32) {}
        };
        AZStd::vector<BoneInfo> m_bones; /**< The array of bone information used for pre-calculation. */

        /**
         * Skin a part of the mesh.
         * @param mesh The mesh to be skinned.
         * @param startVertex The start vertex index to start skinning.
         * @param endVertex The end vertex index for the range to be skinned.
         * @param boneInfos The pre-calculated skinning matrices shared across the skinning process.
         */
        static void SkinRange(Mesh* mesh, AZ::u32 startVertex, AZ::u32 endVertex, const AZStd::vector<BoneInfo>& boneInfos);

        //! Number of vertices per batch/job used for multi-threaded software skinning.
        static constexpr AZ::u32 s_numVerticesPerBatch = 10000;

        /**
         * Default constructor.
         * @param mesh A pointer to the mesh to deform.
         */
        DualQuatSkinDeformer(Mesh* mesh);

        /**
         * Destructor.
         */
        virtual ~DualQuatSkinDeformer();

        /**
         * Find the entry number that uses a specified node number.
         * @param nodeIndex The node number to search for.
         * @result The index inside the mBones member array, which uses the given node.
         */
        AZ::Outcome<size_t> FindLocalBoneIndex(uint32 nodeIndex) const;
    };
} // namespace EMotionFX
