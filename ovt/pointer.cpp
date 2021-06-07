/*********************************************************
 * Copyright (C) 2006-2019 VMware, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation version 2.1 and no later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the Lesser GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA.
 *
 *********************************************************/

/*
 * pointer.cpp --
 *
 *    Pointer functions.
 */

#include "cp/copyPasteCompat.h"

#include "pointer.h"
#include "vm_assert.h"
#include "backdoor_def.h"

extern "C" {
   #include "backdoor.h"
   #include "rpcvmx.h"
}
#include <cstdint>

//static Bool mouseIsGrabbed;
static AbsoluteMouseState absoluteMouseState = ABSMOUSE_UNKNOWN;
//static uint8_t gHostClipboardTries = 0;

//static void PointerGrabbed(void);
//static void PointerUngrabbed(void);
//static gboolean PointerUpdatePointerLoop(gpointer clientData);

#define POINTER_UPDATE_TIMEOUT 100


/*
 *----------------------------------------------------------------------------
 *
 * PointerGetAbsoluteMouseState
 *
 *    Are the host/guest capable of using absolute mouse mode?
 *
 * Results:
 *    TRUE if host is in absolute mouse mode, FALSE otherwise.
 *
 * Side effects:
 *    Issues Tools RPC.
 *
 *----------------------------------------------------------------------------
 */

AbsoluteMouseState
PointerGetAbsoluteMouseState(void)
{
   Backdoor_proto bp;
   AbsoluteMouseState state = ABSMOUSE_UNKNOWN;

   bp.in.cx.halfs.low = BDOOR_CMD_ISMOUSEABSOLUTE;
   Backdoor(&bp);
   if (bp.out.ax.word == 0) {
      state = ABSMOUSE_UNAVAILABLE;
   } else if (bp.out.ax.word == 1) {
      state = ABSMOUSE_AVAILABLE;
   }

   return state;
}

/*
 *-----------------------------------------------------------------------------
 *
 * PointerGetPos --
 *
 *      Retrieve the host notion of the guest pointer location.
 *
 * Results:
 *      '*x' and '*y' are the coordinates (top left corner is 0, 0) of the
 *      host notion of the guest pointer location. (-100, -100) means that the
 *      mouse is not grabbed on the host.
 *
 * Side effects:
 *	None
 *
 *-----------------------------------------------------------------------------
 */
void PointerGetPos(int16_t *x, // OUT
              int16_t *y) // OUT
{
   Backdoor_proto bp;

   bp.in.cx.halfs.low = BDOOR_CMD_GETPTRLOCATION;
   Backdoor(&bp);
   *x = bp.out.ax.word >> 16;
   *y = bp.out.ax.word;
}


/*
 *-----------------------------------------------------------------------------
 *
 * PointerSetPos --
 *
 *      Update the host notion of the guest pointer location. 'x' and 'y' are
 *      the coordinates (top left corner is 0, 0).
 *
 * Results:
 *      None
 *
 * Side effects:
 *	None
 *
 *-----------------------------------------------------------------------------
 */

void PointerSetPos(uint16_t x,
		   uint16_t y) {
   Backdoor_proto bp;

   bp.in.cx.halfs.low = BDOOR_CMD_SETPTRLOCATION;
   bp.in.size = (uint32_t) ((x << 16) | y);
   Backdoor(&bp);
}


#if 0
/*
 *-----------------------------------------------------------------------------
 *
 * PointerGrabbed --
 *
 *      Called when the pointer's state switches from released to grabbed.
 *      We warp the cursor to whatever position the vmx tells us, and then
 *      setup the loop which attempts to get the host clipboard.
 *
 * Result:
 *      None..
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

static void
PointerGrabbed(void)
{
   short hostPosX;
   short hostPosY;

   PointerGetPos(&hostPosX, &hostPosY);
   // TODO: Set pointer position here.
   gHostClipboardTries = 9;
}


/*
 *-----------------------------------------------------------------------------
 *
 * PointerUngrabbed --
 *
 *    Called by the background thread when the pointer's state switches
 *    from grabbed to ungrabbed
 *
 * Result:
 *      None..
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

static void
PointerUngrabbed()
{
   CopyPaste_RequestSelection();
}


/*
 *-----------------------------------------------------------------------------
 *
 * PointerUpdatePointerLoop  --
 *
 *      Event Manager function for tracking the mouse/pointer/clipboard state.
 *      Manage grabbed/ungrab state based on x/y data from backdoor. On the
 *      transition to grabbed, call PointerHasBeenGrabbed(). While grabbed,
 *      send guest pointer coordinates thru the backdoor. Also, make several
 *      attempts to get the host clipboard from the backdoor. When changing
 *      to ungrabbed, call PointerHasBeenUngrabbed, which will push our
 *      clipboard thru the backdoor. While ungrabbed, don't do a thing.
 *
 *      This function is queued in Event Manager only when vmx doesn't support
 *      RPC copy/paste because newer vmx initiates copy/paste from UI through
 *      RPC, and doesn't need cursor grab/ungrab state to start copy/paste.
 *
 * Results:
 *      FALSE.
 *
 * Side effects:
 *      Lots. The vmx's notion of guest cursor position could change, the
 *      vmx's clipboard could change, and the guest's clipboard could change.
 *
 *-----------------------------------------------------------------------------
 */

static gboolean
PointerUpdatePointerLoop(gpointer clientData) // IN: unused
{
   int16 hostPosX, hostPosY;
   int guestX, guestY;

   PointerGetPos(&hostPosX, &hostPosY);
   if (mouseIsGrabbed) {
      if (hostPosX == UNGRABBED_POS) {
         /* We transitioned from grabbed to ungrabbed */
         mouseIsGrabbed = FALSE;
         g_debug("PointerUpdatePointerLoop: ungrabbed\n");
         PointerUngrabbed();
      } else {
	// TODO GetCursorPos from the guest
	/*
	 * Report the new guest pointer location (It is used to teach VMware
	 * where to position the outside pointer if the user releases the guest
	 * pointer via the key combination).
	 */
	// TODO set the host notion of cursor position
#if defined(WIN32)

         /*
          * We used to return early if GetCursorPos() failed, but we at least want to
          * continue and do the selection work. Also, I'm not sure we need this code anymore
          * since all new tools have absolute pointing device
          */
         if (!GetCursorPos(&guestPos)) {
            g_debug("PointerIsGrabbed: GetCursorPos() failed!\n");
         } else {
            if ( hostPosX != guestPos.x || hostPosY != guestPos.y) {
               /*
                * Report the new guest pointer location (It is used to teach VMware
                * where to position the outside pointer if the user releases the guest
                * pointer via the key combination).
                */
               PointerSetPos(guestPos.x, guestPos.y);
            }
         }
#endif
      }
   } else {
      if (hostPosX != UNGRABBED_POS) {
         mouseIsGrabbed = TRUE;
         g_debug("PointerUpdatePointerLoop: grabbed\n");
         PointerGrabbed();
      }
   }

   if (!CopyPaste_IsRpcCPSupported() ||
       (absoluteMouseState == ABSMOUSE_UNAVAILABLE)) {
      CopyPasteDnDWrapper *wrapper = CopyPasteDnDWrapper::GetInstance();
      ToolsAppCtx *ctx = wrapper->GetToolsAppCtx();

      if (ctx) {
         GSource *src = VMTools_CreateTimer(POINTER_UPDATE_TIMEOUT);

         VMTOOLSAPP_ATTACH_SOURCE(ctx, src, PointerUpdatePointerLoop, NULL, NULL);
         g_source_unref(src);
      }
   }

   return FALSE;
}


#endif // 0
