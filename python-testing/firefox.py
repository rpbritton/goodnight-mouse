#!/usr/bin/env python

# from simple_at import *
from pyatspi import StateSet

# Ariel Rios
# ariel@gnu.org
# September 29, 2007

def run(acc):
  print("[start]")

  col = acc.queryInterface('IDL:Accessibility/Collection:1.0')

  if col is None:
    raise NotImplementedError('%s does not implement collection' % acc.name)

  print("[Firefox test]")

  states = StateSet()

  rule = col.createMatchRule(states.raw(), col.MATCH_ANY,
                             "", col.MATCH_ALL,
                              [Accessibility.ROLE_FRAME], col.MATCH_ANY,
                             "IDL:Accessibility/Document:1.0", col.MATCH_ALL,
                             False)
  ls = col.getMatches (rule, col.SORT_ORDER_CANONICAL, 0)

  frame = ls[0]
  fcol = frame.queryInterface ('IDL:Accessibility/Collection:1.0')

  frule = fcol.createMatchRule(states.raw(), col.MATCH_ANY,
                              "", col.MATCH_ALL,
                              "", col.MATCH_ALL,
                              "IDL:Accessibility/Collection:1.0", col.MATCH_ALL,
                              False)
  fls = fcol.getMatches (frule, fcol.SORT_ORDER_CANONICAL, 0)

  print("[Opened documents]")

  for n in fls:
    print("\t", n.name)
    if n.getState().contains (Accessibility.STATE_VISIBLE):
      frame = n

  print("[Visible document]")

  print("\t", frame.name)


  doc = frame
  hcol = doc.queryInterface ('IDL:Accessibility/Collection:1.0')

  hrule = hcol.createMatchRule (states.raw(), col.MATCH_ALL,
                                ["tag:H2"], col.MATCH_ANY,
                                "", col.MATCH_ALL,
                                "IDL:Accessibility/Hyperlink:1.0", col.MATCH_ALL,
                                False)

  hls = hcol.getMatches (hrule, hcol.SORT_ORDER_CANONICAL, 0)

  print("[Headers]")

  for header in hls:
    print("\t", header.queryInterface("IDL:Accessibility/EditableText:1.0").getText(0, -1))

  dcol = doc.queryInterface ('IDL:Accessibility/Collection:1.0')

  drule = dcol.createMatchRule (states.raw(), col.MATCH_ANY,
                                "", col.MATCH_ALL,
                                "", col.MATCH_ALL,
                                "IDL:Accessibility/Hyperlink:1.0;IDL:Accessibility/Action:1.0", col.MATCH_ALL,
                                False)

  dls = dcol.getMatches (drule,dcol.SORT_ORDER_CANONICAL, 0)

  print("[Hyperlinks]")

  for n in dls:
    print("\t", n.name)#, n.accessible.getAttributes()

  #col.freeMatchRule(rule)
  #dcol.freeMatchRule(hrule)
  #dcol.freeMatchRule(drule)
  return 1

if __name__ == '__main__':
  d = registry.getDesktop(0)
  for i in xrange(d.childCount):
    try:
      c = d.getChildAtIndex(i)
    except:
      continue
    if c is not None and c.name == 'Gran Paradiso':
      run(c)

print("[fin]")
