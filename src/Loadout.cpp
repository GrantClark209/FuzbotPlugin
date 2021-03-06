#include "Loadout.h"

#include "FuzbotPlugin.h"

// Vanilla Item

Item::Item(ProductWrapper prod, OnlineProductWrapper o_prod, Game game) {
  data["id"] = prod.GetID();
  data["name"] = prod.GetLongLabel().ToString();
  data["quality"] = BM::QUALITY[prod.GetQuality()];

  if (o_prod)
    for (ProductAttributeWrapper attr : o_prod.GetAttributes())
      setAttribute(attr, game);
}

void Item::setAttribute(ProductAttributeWrapper attr, Game game) {
  std::string type = attr.GetTypename();
  uintptr_t mem_addr = attr.memory_address;
  ItemsWrapper iw = game->GetItemsWrapper();
  if (type == "Certified") return setCertified(mem_addr, iw);
  if (type == "Painted") return setPainted(mem_addr, iw);
  if (type == "SpecialEdition") return setSpecialEdition(mem_addr, iw);
  if (type == "TeamEdition") return setTeamEdition(mem_addr, iw);
}

void Item::setCertified(uintptr_t mem_addr, ItemsWrapper iw) {
  data["certified"] = iw.GetCertifiedStatDB().GetStatName(
      ProductAttribute_CertifiedWrapper(mem_addr).GetStatId());
}

void Item::setPainted(uintptr_t mem_addr, ItemsWrapper iw) {
  data["painted"] = iw.GetPaintDB().GetPaintName(
      ProductAttribute_PaintedWrapper(mem_addr).GetPaintID());
}

void Item::setSpecialEdition(uintptr_t mem_addr, ItemsWrapper iw) {
  std::string edition = iw.GetSpecialEditionDB().GetSpecialEditionName(
      ProductAttribute_SpecialEditionWrapper(mem_addr).GetEditionID());
  data["special_edition"] = edition.substr(edition.find("_") + 1);
}

void Item::setTeamEdition(uintptr_t mem_addr, ItemsWrapper iw) {
  data["team_edition"] = iw.GetEsportTeamDB().GetName(
      ProductAttribute_TeamEditionWrapper(mem_addr).GetId());
}

// Vanilla Loadout

Loadout::Loadout(bool isOrange, CvarManager cvarManager, Game game)
    : cvarManager(cvarManager), game(game) {
  ItemsWrapper iw = game->GetItemsWrapper();
  if (iw.IsNull()) {
    cvarManager->log("Unable to load Rocket League Loadout");
    return;
  }

  LoadoutWrapper lw = iw.GetCurrentLoadout(isOrange);
  // TODO: Remove below when BakkesMod updates
  lw = LoadoutWrapper(*reinterpret_cast<uintptr_t*>(lw.memory_address));

  bool foundPrimary = false;

  for (uint64_t id : lw.GetOnlineLoadout()) {
    if (id == 0) continue;  // 0 indicates no data for that slot
    OnlineProductWrapper onlineProduct = iw.GetOnlineProduct(id);

    // If the id does not match an online product, cast unsigned long long
    // (uint64) -> int to check as a productId.
    bool isValidInt = id - INT_MIN <= (uint64_t)INT_MAX - INT_MIN;
    if (!onlineProduct && !isValidInt) {
      cvarManager->log("Missing Online ID: " + std::to_string(id));
      continue;
    }

    ProductWrapper product =
        onlineProduct ? onlineProduct.GetProduct() : iw.GetProduct((int)id);

    if (!product) {
      cvarManager->log("ERROR: Product is null!");
      continue;
    }

    ProductSlotWrapper productSlot = product.GetSlot();
    if (productSlot.IsNull()) {
      cvarManager->log("ERROR: Slot is null!");
      continue;
    }

    int slotIdx = productSlot.GetSlotIndex();
    // Assign second "finish" slot to accent/secondary finish
    if (slotIdx == 7) {
      if (foundPrimary) {
        slotIdx = 12;
      } else {
        foundPrimary = true;
      }
    }
    // Validate slotIdx against our defined SLOT map
    if (slotIdx < 0 || slotIdx >= BM::SLOT.size()) {
      cvarManager->log("Invalid Slot Index: " + std::to_string(slotIdx));
      cvarManager->log(
          "Plugin may be outdated. New items categories might not be stored.");
      continue;
    }

    std::string slot = BM::SLOT[slotIdx];
    Item item(product, onlineProduct, game);
    items[slot] = item.data;
  }
};

// BakkesMod Item

Item::Item(BM::Item bmItem, ItemsWrapper iw) {
  ProductWrapper prod = iw.GetProduct(bmItem.productId);
  data["id"] = bmItem.productId;
  data["name"] = prod.GetLongLabel().ToString();
  data["quality"] = BM::QUALITY[prod.GetQuality()];
  data["painted"] = iw.GetPaintDB().GetPaintName(bmItem.paintId);
}

// BakkesMod Loadout

Loadout::Loadout(bool isOrange, BM::Teams teams, CvarManager cvarManager,
                 Game game)
    : cvarManager(cvarManager), game(game) {
  ItemsWrapper iw = game->GetItemsWrapper();
  for (BM::Item bmItem : isOrange ? teams.orangeItems : teams.blueItems) {
    std::string slot = BM::SLOT[bmItem.slotIdx];
    Item item(bmItem, iw);
    items[slot] = item.data;
  }
  json color = isOrange ? teams.orangeColor : teams.blueColor;
  if (color != nullptr) {
    items["primary_finish"]["painted"] = color["primary"];
    items["accent_finish"]["painted"] = color["accent"];
  }
};

void to_json(json& j, const Item& item) { j = item.data; }
void to_json(json& j, const Loadout& loadout) { j = loadout.items; }